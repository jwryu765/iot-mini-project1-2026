#pragma once
#include <mysql.h>
#include <vector>
#include <string>
#include "Word.h"

// DBManager 클래스는 MySQL 데이터베이스와의 연결 및 쿼리 실행을 담당하는 클래스입니다.
class DBManager {
private:
    MYSQL* conn;
    std::string escapeString(const std::string& input);

public:
    DBManager();
    ~DBManager();

	// DB 연결 함수입니다. 연결 성공 시 UTF-8 문자셋을 설정하여 한글이 깨지지 않도록 했습니다. 실패 시 에러 메시지를 출력합니다.
    bool connect();

    // 단어 목록을 불러오는 함수입니다. 이제 '누구의 데이터인지(username)'를 요구하기 때문에, LEFT JOIN으로 전체 단어와 사용자의 학습 기록을 병합하여 가져옵니다.
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