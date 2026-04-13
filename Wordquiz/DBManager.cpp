#include "DBManager.h"
#include <iostream>

using namespace std;

DBManager::DBManager() {
    conn = mysql_init(NULL);
}

DBManager::~DBManager() {
    if (conn) mysql_close(conn);
}

string DBManager::escapeString(const std::string& input) {
    if (input.empty()) return "";
    char* to = new char[input.length() * 2 + 1];
    mysql_real_escape_string(conn, to, input.c_str(), (unsigned long)input.length());
    string result(to);
    delete[] to;
    return result;
}

bool DBManager::connect() {
    // 본인의 DB 비밀번호로 수정하세요
    if (mysql_real_connect(conn, "127.0.0.1", "root", "my123456", "Wordquiz", 3306, NULL, 0)) {
        mysql_query(conn, "set names utf8mb4");
        return true;
    }
    cout << "MySQL 연결 에러: " << mysql_error(conn) << endl;
    return false;
}

// ✨ [v6.1 핵심] 단어 목록 + 내 개인 진척도를 합쳐서(LEFT JOIN) 가져오는 마법의 쿼리
vector<Word> DBManager::loadAllWords(const string& username) {
    vector<Word> list;
    string safeUser = escapeString(username);

    // words(전체 사전)와 user_words(내 기록)를 병합. 기록이 없으면 0으로 가져옴(IFNULL)
    string query = "SELECT w.id, w.word, w.meaning, "
        "IFNULL(uw.level, 0), IFNULL(uw.failCount, 0), IFNULL(uw.nextReview, 0) "
        "FROM words w "
        "LEFT JOIN user_words uw ON w.word = uw.word AND uw.username = '" + safeUser + "'";

    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res))) {
            list.push_back(Word(
                stoi(row[0]), row[1], row[2],
                stoi(row[3]), stoi(row[4]), stoll(row[5])
            ));
        }
        mysql_free_result(res);
    }
    return list;
}

bool DBManager::insertWord(const string& eng, const string& kor) {
    string safeEng = escapeString(eng);
    string safeKor = escapeString(kor);

    string query = "INSERT INTO words (word, meaning) VALUES ('" + safeEng + "', '" + safeKor + "')";
    if (mysql_query(conn, query.c_str()) == 0) return true;
    return false;
}

// ✨ [v6.1 핵심] 내 단어 진척도를 저장. 없으면 새로 만들고(INSERT), 있으면 덮어씌움(UPDATE)
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

// ✨ (추가로 resetAllWordStats 도 이걸로 바꿔주세요)
bool DBManager::resetAllWordStats() {
    // 이제 초기화는 개인 기록 테이블을 통째로 비워버리는 방식입니다.
    if (mysql_query(conn, "TRUNCATE TABLE user_words") == 0) return true;
    return false;
}

// ✨ [v5.4 관리자] 특정 단어 완전 삭제 쿼리
bool DBManager::deleteWord(const string& eng) {
    string safeEng = escapeString(eng);
    string query = "DELETE FROM words WHERE word = '" + safeEng + "'";

    if (mysql_query(conn, query.c_str()) == 0) {
        // mysql_affected_rows: 삭제된 줄이 1개 이상이면 삭제 성공, 0이면 없는 단어
        if (mysql_affected_rows(conn) > 0) return true;
    }
    return false;
}
// =====================================================================
// ✨ [v6.0 추가] 유저 시스템 쿼리문
// =====================================================================

// 회원가입 (INSERT)
bool DBManager::registerUser(const string& username, const string& password) {
    string safeUser = escapeString(username);
    string safePw = escapeString(password);
    string query = "INSERT INTO users (username, password, score) VALUES ('" + safeUser + "', '" + safePw + "', 0)";

    if (mysql_query(conn, query.c_str()) == 0) return true;
    return false; // 이미 있는 아이디면 실패함 (UNIQUE 제약조건)
}

// 로그인 (SELECT)
bool DBManager::loginUser(const string& username, const string& password) {
    string safeUser = escapeString(username);
    string safePw = escapeString(password);
    string query = "SELECT id FROM users WHERE username = '" + safeUser + "' AND password = '" + safePw + "'";

    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            int rows = mysql_num_rows(res); // 일치하는 정보가 있는지 확인
            mysql_free_result(res);
            return rows > 0;
        }
    }
    return false;
}

// 점수 올리기 (UPDATE)
bool DBManager::addScore(const string& username, int points) {
    string safeUser = escapeString(username);
    string query = "UPDATE users SET score = score + " + to_string(points) + " WHERE username = '" + safeUser + "'";
    if (mysql_query(conn, query.c_str()) == 0) return true;
    return false;
}

// 명예의 전당 정보 가져오기 (SELECT & ORDER BY)
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
bool DBManager::resetMemberStats() {
    // 테이블 이름은 'users', 초기화할 컬럼은 'score' 하나뿐입니다.
    string query = "UPDATE users SET score = 0";

    if (mysql_query(conn, query.c_str()) == 0) {
        return true;
    }
    else {
        // 에러 발생 시 원인 출력
        cout << "❌ 회원 스탯 초기화 오류: " << mysql_error(conn) << endl;
        return false;
    }
}