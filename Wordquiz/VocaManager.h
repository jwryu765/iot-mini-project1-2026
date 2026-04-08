#pragma once
#include <vector>
#include <string>
#include "Word.h"

using namespace std;

class VocaManager {
private:
    vector<Word> wordList;

public:
    void loadFromCSV(const string& filename); // CSV 불러오기
    void addWord();                           // 수동 추가
    void showUnknownWords();                  // 모르는 단어장 보기
    void runQuiz(bool isIntensive);           // 퀴즈 실행 (true: 집중, false: 일반)
};
