#pragma once
#include <string>
using namespace std;

class Word {
public:
    string english;
    string korean;
    int level;      // 0: 모름, 1: 학습중, 2: 암기완료
    int failCount;  // 오답 횟수

    Word(string eng, string kor, int lvl = 0, int fail = 0)
        : english(eng), korean(kor), level(lvl), failCount(fail) {
    }
};