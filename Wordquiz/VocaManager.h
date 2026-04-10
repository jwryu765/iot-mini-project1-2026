#pragma once
#include <vector>
#include <string>
#include "Word.h"
#include "DBManager.h" 

class VocaManager {
private:
    std::vector<Word> wordList; // std:: 추가
    DBManager db;

public:
    bool init();
    void addWord();
    void showAllWords();
    void showUnknownWords();
    void runQuiz(bool isIntensive);
};