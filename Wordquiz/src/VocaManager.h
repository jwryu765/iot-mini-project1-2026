#pragma once
#include <vector>
#include <string>
#include "Word.h"
#include "DBManager.h" 

// VocaManager 클래스는 단어장 관리, 퀴즈 실행, 사용자 인터페이스 등을 담당하는 클래스입니다.
// DBManager를 통해 데이터베이스와 상호작용하며, 사용자 인증과 다양한 기능을 제공합니다.
class VocaManager {
private:
    std::vector<Word> wordList;
    DBManager db;
    std::string currentUser;

public:
    bool init();
    void showStartScreen();
    bool showAuthMenu();
    void showRanking();
    void showStatistics();
    void showReviewList();     // '모르는 단어장' -> '오답 단어 몰아보기' 로 이름 바뀐 부분
    void addWord();
    void loadFromCSV();        // ✨ [v6.2 추가] CSV 대량 로드 함수 선언
    void showAllWords();
    void runQuiz(bool isIntensive);
    void showAdminMenu();
};