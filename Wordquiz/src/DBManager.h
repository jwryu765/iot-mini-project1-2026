#pragma once
#include <mysql.h>
#include <vector>
#include <string>
#include "Word.h"

class DBManager {
private:
    MYSQL* conn;
    std::string escapeString(const std::string& input);

public:
    DBManager();
    ~DBManager();

    bool connect();

    // ✨ [수정] 이제 데이터를 불러오고 업데이트할 때 '누구의 데이터인지(username)'를 요구합니다.
    std::vector<Word> loadAllWords(const std::string& username);
    bool updateWordStats(const Word& w, const std::string& username);

    bool insertWord(const std::string& eng, const std::string& kor);
    bool resetAllWordStats();
    bool deleteWord(const std::string& eng);
    // ✨ [관리자용] 모든 유저의 점수와 레벨을 초기화하는 함수 추가
    std::vector<Word> searchWords(const std::string& keyword);
    bool deleteUser(const std::string& targetId);
    bool resetMemberStats();
    bool registerUser(const std::string& username, const std::string& password);
    bool loginUser(const std::string& username, const std::string& password);
    bool addScore(const std::string& username, int points);
    std::vector<std::pair<std::string, int>> getTopUsers();
};