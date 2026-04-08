#pragma once
#include <string>

using namespace std;

class Word {
public:
    string english;
    string korean;
    int level;      // 0: 모름, 1: 학습중, 2: 암기완료
    int failCount;  // 오답 횟수

    // 생성자 (여기서 바로 알맹이까지 구현합니다)
    Word(string eng, string kor, int lvl = 0, int fail = 0)
        : english(eng), korean(kor), level(lvl), failCount(fail) {
    }
};
