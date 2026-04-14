#pragma once
#include <string>
#include <ctime>

class Word {
public:
    int id;
    std::string english;
    std::string korean;
    int level;
    int failCount;
    long long nextReview;

    Word(int id, std::string eng, std::string kor, int lvl = 0, int fail = 0, long long next = 0)
        : id(id), english(eng), korean(kor), level(lvl), failCount(fail), nextReview(next) {
        if (next == 0) nextReview = time(0);
    }
};