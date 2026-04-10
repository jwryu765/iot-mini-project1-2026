#include "DBManager.h"
#include <iostream>

using namespace std;

// 생성자: MySQL 객체 초기화
DBManager::DBManager() {
    conn = mysql_init(NULL);
}

// 소멸자: 프로그램 종료 시 연결 해제
DBManager::~DBManager() {
    if (conn) mysql_close(conn);
}

// DB 연결

bool DBManager::connect() {
    if (mysql_real_connect(conn, "127.0.0.1", "root", "my123456", "Wordquiz", 3306, NULL, 0)) {
        // [수정됨] euckr 대신 utf8mb4를 사용하여 한글과 이모지 깨짐을 완벽히 방지합니다.
        mysql_query(conn, "set names utf8mb4");
        return true;
    }
    cout << "MySQL 연결 에러: " << mysql_error(conn) << endl;
    return false;
}

// 전체 단어 불러오기 (SELECT)
vector<Word> DBManager::loadAllWords() {
    vector<Word> list;
    if (mysql_query(conn, "SELECT id, word, meaning, level, failCount, nextReview FROM words") == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res))) {
            // DB에서 가져온 문자열 데이터를 각각의 타입(int, long long)으로 변환
            list.push_back(Word(
                stoi(row[0]),
                row[1],
                row[2],
                stoi(row[3]),
                stoi(row[4]),
                stoll(row[5])
            ));
        }
        mysql_free_result(res);
    }
    return list;
}

// 단어 추가 (INSERT)
bool DBManager::insertWord(const string& eng, const string& kor) {
    string query = "INSERT INTO words (word, meaning) VALUES ('" + eng + "', '" + kor + "')";
    if (mysql_query(conn, query.c_str()) == 0) {
        return true;
    }
    cout << "단어 추가 에러: " << mysql_error(conn) << endl;
    return false;
}

// 단어 상태 업데이트 (UPDATE) - 퀴즈 풀 때마다 호출됨
bool DBManager::updateWordStats(const Word& w) {
    string query = "UPDATE words SET level = " + to_string(w.level) +
        ", failCount = " + to_string(w.failCount) +
        ", nextReview = " + to_string(w.nextReview) +
        " WHERE id = " + to_string(w.id);

    if (mysql_query(conn, query.c_str()) == 0) {
        return true;
    }
    return false;
}