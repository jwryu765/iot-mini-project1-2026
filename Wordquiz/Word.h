#pragma once
#include <string>
#include <ctime>

using namespace std;

class Word {
public:
    string english;
    string korean;
    int level;          // 0~5단계 (높을수록 장기 기억)
    int failCount;      // 총 오답 횟수
    long long nextReview; // 다음 복습 가능 시간 (Unix Timestamp)

    Word(string eng, string kor, int lvl = 0, int fail = 0, long long next = 0)
        : english(eng), korean(kor), level(lvl), failCount(fail), nextReview(next) {
        if (next == 0) nextReview = time(0); // 생성 시 즉시 복습 가능
    }
};