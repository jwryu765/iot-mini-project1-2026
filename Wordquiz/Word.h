#pragma once
#include <string>
#include <ctime>

class Word {
public:
    int id;               // DB의 고유 번호 (Primary Key)
    std::string english;  // std:: 추가
    std::string korean;   // std:: 추가
    int level;
    int failCount;
    long long nextReview;

    Word(int id, std::string eng, std::string kor, int lvl = 0, int fail = 0, long long next = 0)
        : id(id), english(eng), korean(kor), level(lvl), failCount(fail), nextReview(next) {
        if (next == 0) nextReview = time(0);
    }
};