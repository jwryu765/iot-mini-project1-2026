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
    if (mysql_real_connect(conn, "127.0.0.1", "root", "my123456", "Wordquiz", 3306, NULL, 0)) {
        mysql_query(conn, "set names utf8mb4");
        return true;
    }
    cout << "MySQL 연결 에러: " << mysql_error(conn) << endl;
    return false;
}

/*
 * [데이터 병합 및 SQL 최적화]
 * 전체 사전(words)과 사용자의 개별 학습 기록(user_words)을 병합합니다.
 * 학습 기록이 없는 '새 단어'도 목록에 나와야 하므로 INNER JOIN이 아닌 'LEFT JOIN'을 사용했습니다.
 * 매칭되는 기록이 없어 NULL이 발생하는 컬럼은 IFNULL()로 0 처리하여,
 * C++ 객체 매핑 시 런타임 에러(NULL 포인터 참조 등)가 발생하지 않도록 DB단에서 선제적으로 방어했습니다.
 */
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

bool DBManager::resetAllWordStats() {
    if (mysql_query(conn, "TRUNCATE TABLE user_words") == 0) return true;
    return false;
}

bool DBManager::deleteWord(const string& eng) {
    string safeEng = escapeString(eng);
    string query = "DELETE FROM words WHERE word = '" + safeEng + "'";

    if (mysql_query(conn, query.c_str()) == 0) {
        if (mysql_affected_rows(conn) > 0) return true;
    }
    return false;
}

bool DBManager::registerUser(const string& username, const string& password) {
    string safeUser = escapeString(username);
    string safePw = escapeString(password);
    string query = "INSERT INTO users (username, password, score) VALUES ('" + safeUser + "', '" + safePw + "', 0)";

    if (mysql_query(conn, query.c_str()) == 0) return true;
    return false;
}

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

bool DBManager::addScore(const string& username, int points) {
    string safeUser = escapeString(username);
    string query = "UPDATE users SET score = score + " + to_string(points) + " WHERE username = '" + safeUser + "'";
    if (mysql_query(conn, query.c_str()) == 0) return true;
    return false;
}

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
    string query = "UPDATE users SET score = 0";
    if (mysql_query(conn, query.c_str()) == 0) {
        return true;
    }
    else {
        cout << "❌ 회원 스탯 초기화 오류: " << mysql_error(conn) << endl;
        return false;
    }
}

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