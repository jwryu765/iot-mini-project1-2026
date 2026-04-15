#pragma once
#include <string>
#include <ctime>

// Word 클래스는 단어의 정보를 담는 데이터 구조입니다. DB에서 불러온 데이터를 이 클래스의 객체로 매핑하여 사용합니다.
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