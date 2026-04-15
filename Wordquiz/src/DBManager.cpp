#include "DBManager.h"
#include <iostream>
#include <vector>

using namespace std;

// [DB 연결 및 SQL 인젝션 방어]
DBManager::DBManager() {
    conn = mysql_init(NULL);
}

// DB 연결 종료 시 mysql_close()로 자원 해제. 프로그램 종료 시 자동으로 호출됩니다.
DBManager::~DBManager() {
    if (conn) mysql_close(conn);
}

string DBManager::escapeString(const std::string& input) {
    if (input.empty()) return "";

    // 최적화: new char[] 대신 vector를 사용하여 함수 종료 시 메모리가 자동(안전하게) 해제되도록 설계
    vector<char> buffer(input.length() * 2 + 1);

    mysql_real_escape_string(conn, buffer.data(), input.c_str(), (unsigned long)input.length());

    return string(buffer.data());
}

// MySQL 서버에 연결하는 함수입니다. 연결 성공 시 UTF-8 문자셋을 설정하여 한글이 깨지지 않도록 했습니다. 실패 시 에러 메시지를 출력합니다.
bool DBManager::connect() {
    if (mysql_real_connect(conn, "127.0.0.1", "root", "my123456", "Wordquiz", 3306, NULL, 0)) {
        mysql_query(conn, "set names utf8mb4");
        return true;
    }
    cout << "MySQL 연결 에러: " << mysql_error(conn) << endl;
    return false;
}

/*
[데이터 병합 및 SQL 최적화]
전체 사전(words)과 사용자의 개별 학습 기록(user_words)을 병합합니다.
학습 기록이 없는 '새 단어'도 목록에 나와야 하므로 INNER JOIN이 아닌 'LEFT JOIN'을 사용했습니다.
매칭되는 기록이 없어 NULL이 발생하는 컬럼은 IFNULL()로 0 처리하여,
C++ 객체 매핑 시 런타임 에러(NULL 포인터 참조 등)가 발생하지 않도록 DB단에서 선제적으로 방어했습니다.
*/

// 단어 목록을 불러오는 함수입니다. 이제 '누구의 데이터인지(username)'를 요구하기 때문에, LEFT JOIN으로 전체 단어와 사용자의 학습 기록을 병합하여 가져옵니다.
// 학습 기록이 없는 단어는 레벨과 오답 횟수, 복습 시기를 0으로 처리하여 반환합니다.
vector<Word> DBManager::loadAllWords(const string& username) {
    vector<Word> list;
    string safeUser = escapeString(username);

    string query = "SELECT w.id, w.word, w.meaning, "
        "IFNULL(uw.level, 0), IFNULL(uw.failCount, 0), IFNULL(uw.nextReview, 0) "
        "FROM words w "
        "LEFT JOIN user_words uw ON w.word = uw.word AND uw.username = '" + safeUser + "'";

    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res))) {
            // 최적화: 삼항 연산자를 사용해 DB에서 NULL 값이 넘어오더라도 프로그램이 뻗지 않고 0이나 빈 문자열로 안전하게 초기화되도록 방어적 프로그래밍 적용
            list.push_back(Word(
                row[0] ? stoi(row[0]) : 0,           // id
                row[1] ? row[1] : "",                // english
                row[2] ? row[2] : "",                // korean
                row[3] ? stoi(row[3]) : 0,           // level
                row[4] ? stoi(row[4]) : 0,           // failCount
                row[5] ? stoll(row[5]) : 0           // nextReview
            ));
        }
    }
    return list;
}

// 단어를 추가하는 함수입니다. 사용자 입력을 이스케이프 처리하여 SQL 인젝션 공격을 방어합니다.
bool DBManager::insertWord(const string& eng, const string& kor) {
    string safeEng = escapeString(eng);
    string safeKor = escapeString(kor);

    string query = "INSERT INTO words (word, meaning) VALUES ('" + safeEng + "', '" + safeKor + "')";
    if (mysql_query(conn, query.c_str()) == 0) return true;
    return false;
}

// 단어 학습 기록을 업데이트하는 함수입니다. '누구의 데이터인지(username)'를 요구하기 때문에, ON DUPLICATE KEY UPDATE 구문을 사용하여 기존 기록이 있으면 업데이트, 없으면 새로 삽입하도록 했습니다.
bool DBManager::updateWordStats(const Word& w, const string& username) {
    string safeUser = escapeString(username);
    string safeWord = escapeString(w.english);

    string query = "INSERT INTO user_words (username, word, level, failCount, nextReview) "
        "VALUES ('" + safeUser + "', '" + safeWord + "', " +
        to_string(w.level) + ", " + to_string(w.failCount) + ", " + to_string(w.nextReview) + ") "
        "ON DUPLICATE KEY UPDATE "
        "level = " + to_string(w.level) + ", "
        "failCount = " + to_string(w.failCount) + ", "
        "nextReview = " + to_string(w.nextReview);

    if (mysql_query(conn, query.c_str()) == 0) return true;
    return false;
}

// 모든 단어 학습 기록을 초기화하는 함수입니다. user_words 테이블을 TRUNCATE하여 모든 사용자의 학습 기록을 초기화합니다.
bool DBManager::resetAllWordStats() {
    if (mysql_query(conn, "TRUNCATE TABLE user_words") == 0) return true;
    return false;
}

// 단어를 삭제하는 함수입니다. 단어가 삭제되면, 그 단어에 대한 모든 사용자 학습 기록도 함께 삭제해야 하므로, 트랜잭션을 사용하여 두 테이블에서 일관된 삭제가 이루어지도록 했습니다.
bool DBManager::deleteWord(const string& eng) {
    string safeEng = escapeString(eng);
    string query = "DELETE FROM words WHERE word = '" + safeEng + "'";

    if (mysql_query(conn, query.c_str()) == 0) {
        if (mysql_affected_rows(conn) > 0) return true;
    }
    return false;
}

// 회원 가입과 로그인, 점수 추가, 랭킹 조회 등의 함수입니다. 모두 사용자 입력을 이스케이프 처리하여 SQL 인젝션 공격을 방어합니다.
bool DBManager::registerUser(const string& username, const string& password) {
    string safeUser = escapeString(username);
    string safePw = escapeString(password);
    string query = "INSERT INTO users (username, password, score) VALUES ('" + safeUser + "', '" + safePw + "', 0)";

    if (mysql_query(conn, query.c_str()) == 0) return true;
    return false;
}

// 로그인 함수입니다. 입력된 아이디와 비밀번호가 users 테이블에 존재하는지 확인합니다. SQL 인젝션 공격을 방어하기 위해 입력값을 이스케이프 처리합니다.
bool DBManager::loginUser(const string& username, const string& password) {
    string safeUser = escapeString(username);
    string safePw = escapeString(password);
    string query = "SELECT id FROM users WHERE username = '" + safeUser + "' AND password = '" + safePw + "'";

    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            int rows = mysql_num_rows(res);
            mysql_free_result(res);
            return rows > 0;
        }
    }
    return false;
}

// 점수 추가 함수입니다. 사용자의 점수를 업데이트할 때도 SQL 인젝션 공격을 방어하기 위해 입력값을 이스케이프 처리합니다.
bool DBManager::addScore(const string& username, int points) {
    string safeUser = escapeString(username);
    string query = "UPDATE users SET score = score + " + to_string(points) + " WHERE username = '" + safeUser + "'";
    if (mysql_query(conn, query.c_str()) == 0) return true;
    return false;
}

// 랭킹 조회 함수입니다. users 테이블에서 점수 순으로 상위 5명의 사용자 이름과 점수를 가져옵니다. SQL 인젝션 공격을 방어하기 위해 입력값을 이스케이프 처리합니다.
vector<pair<string, int>> DBManager::getTopUsers() {
    vector<pair<string, int>> list;
    string query = "SELECT username, score FROM users ORDER BY score DESC LIMIT 5";

    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res))) {
            list.push_back({ row[0], stoi(row[1]) });
        }
        mysql_free_result(res);
    }
    return list;
}

// 모든 회원의 점수와 레벨을 초기화하는 함수입니다. users 테이블에서 모든 사용자의 score 컬럼을 0으로 업데이트합니다.
bool DBManager::resetMemberStats() {
    string query = "UPDATE users SET score = 0";
    if (mysql_query(conn, query.c_str()) == 0) {
        return true;
    }
    else {
        cout << "❌ 회원 스탯 초기화 오류: " << mysql_error(conn) << endl;
        return false;
    }
}

// 단어 검색 함수입니다. 입력된 키워드가 단어 또는 뜻에 포함된 경우를 검색합니다. SQL 인젝션 공격을 방어하기 위해 입력값을 이스케이프 처리합니다.
vector<Word> DBManager::searchWords(const string& keyword) {
    vector<Word> results;
    string safeKeyword = escapeString(keyword);

    string query = "SELECT id, word, meaning FROM words WHERE word LIKE '%" + safeKeyword + "%' OR meaning LIKE '%" + safeKeyword + "%'";

    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res))) {
            results.push_back(Word(stoi(row[0]), row[1], row[2], 0, 0, 0));
        }
        mysql_free_result(res);
    }
    return results;
}

// 사용자 삭제 함수입니다. 특정 사용자를 삭제할 때, 그 사용자의 학습 기록도 함께 삭제해야 하므로, 트랜잭션을 사용하여 두 테이블에서 일관된 삭제가 이루어지도록 했습니다.
bool DBManager::deleteUser(const string& targetId) {
    string safeId = escapeString(targetId);

    string query1 = "DELETE FROM user_words WHERE username = '" + safeId + "'";
    mysql_query(conn, query1.c_str());

    string query2 = "DELETE FROM users WHERE username = '" + safeId + "'";
    if (mysql_query(conn, query2.c_str()) == 0) {
        if (mysql_affected_rows(conn) > 0) return true;
    }
    return false;
}