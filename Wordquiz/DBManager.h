#pragma once
#include <mysql.h>
#include <vector>
#include <string>
#include "Word.h"

class DBManager {
private:
    MYSQL* conn;

public:
    DBManager();
    ~DBManager();

    bool connect();                                                // DB 연결
    std::vector<Word> loadAllWords();                              // 전체 단어 불러오기
    bool insertWord(const std::string& eng, const std::string& kor); // 단어 추가
    bool updateWordStats(const Word& w);                           // 퀴즈 결과(레벨, 오답수 등) 업데이트
};