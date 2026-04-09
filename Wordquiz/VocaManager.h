#pragma once
#include <vector>
#include <string>
#include "Word.h"
using namespace std;

class VocaManager {
private:
    vector<Word> wordList;

public:
    void loadFromCSV(const string& filename); // CSV 자동 불러오기
    void saveToCSV(const string& filename);   // [기능 5] CSV 저장하기

    void addWord();                           // 수동 추가
    void showAllWords();                      // [기능 1, 2] 전체 단어장 표로 보기
    void showUnknownWords();                  // 모르는 단어장 보기
    void runQuiz(bool isIntensive);           // 퀴즈 실행 (true: 집중, false: 일반)
};