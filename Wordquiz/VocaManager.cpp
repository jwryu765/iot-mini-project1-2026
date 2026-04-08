/*
    **기능 추가할 것**
    
    1. 전체 단어장과 오답 단어장을 나누기
	2. 단어장에 단어 추가할 때, 이미 존재하는 단어인지 체크하기
	3. 단어장 저장 기능 (CSV로 저장)
	4. 기존 단어장에 새로운 단어 추가할 때, 기존 단어장과 병합하여 저장하기
	5. 콘솔을 끌때 자동 저장 기능 탑재하기 (프로그램 종료 시점에 단어장 저장), 다시 킬때 csv 안불러와도 되게하기
	6. 3번 이상 틀린 단어는 매우 어려운 단어로 분류하여 3번 틀린 단어만 나오는 퀴즈 모드 추가하기
*/

#include "VocaManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm> 
#include <random>    

using namespace std;

// 다중 뜻(쉼표 여러 개) 지원 버전으로 업그레이드된 로드 함수
void VocaManager::loadFromCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "CSV 파일을 찾을 수 없습니다! (voca.csv 파일 위치를 확인해주세요)" << endl;
        return;
    }

    string line;
    wordList.clear();
    int count = 0;

    while (getline(file, line)) {
        if (line.empty()) continue;
        // 윈도우 개행문자(\r) 제거
        if (line.back() == '\r') line.pop_back();

        // 1. 뒤에서부터 쉼표 2개를 찾아 레벨과 오답횟수 위치를 확보합니다.
        size_t lastComma = line.find_last_of(',');
        size_t secondLastComma = (lastComma != string::npos) ? line.find_last_of(',', lastComma - 1) : string::npos;

        // 2. 맨 앞에서 첫 번째 쉼표를 찾아 영단어 위치를 확보합니다.
        size_t firstComma = line.find_first_of(',');

        // 형식이 맞지 않는 줄은 건너뜁니다.
        if (firstComma == string::npos || secondLastComma == string::npos || firstComma >= secondLastComma) continue;

        // 3. 데이터 추출 (핵심!)
        string eng = line.substr(0, firstComma);

        // 3번 항목(또 다른 뜻)까지 포함하여 첫 쉼표와 뒤에서 두번째 쉼표 사이를 통째로 뜻으로 합칩니다.
        string kor = line.substr(firstComma + 1, secondLastComma - firstComma - 1);

        string lvlStr = line.substr(secondLastComma + 1, lastComma - secondLastComma - 1);
        string failStr = line.substr(lastComma + 1);

        try {
            int levelVal = stoi(lvlStr);
            int failVal = stoi(failStr);
            wordList.push_back(Word(eng, kor, levelVal, failVal));
        }
        catch (...) {
            // 숫자가 아닌 값이 들어있을 경우 0으로 기본 세팅하여 튕김 방지
            wordList.push_back(Word(eng, kor, 0, 0));
        }
    }
    file.close();
    cout << "🎉 총 " << wordList.size() << "개의 단어를 성공적으로 불러왔습니다!" << endl;
}

void VocaManager::addWord() {
    string eng, kor;
    cout << "\n[단어 수동 추가]" << endl;
    cout << "영어: "; cin >> eng;
    cout << "한글: "; cin >> kor;
    wordList.push_back(Word(eng, kor));
    cout << "✅ 추가 완료!" << endl;
}

void VocaManager::showUnknownWords() {
    cout << "\n=================================" << endl;
    cout << "        📖 모르는 단어장        " << endl;
    cout << "=================================" << endl;

    int count = 0;
    for (const auto& w : wordList) {
        if (w.level < 2) {
            cout << "- " << w.english << " : " << w.korean
                << " (오답 횟수: " << w.failCount << ")" << endl;
            count++;
        }
    }

    if (count == 0) {
        cout << "축하합니다! 모든 단어를 마스터하셨습니다!" << endl;
    }
    else {
        cout << "\n현재 복습이 필요한 단어: " << count << "개" << endl;
    }
    cout << "=================================\n" << endl;
}

void VocaManager::runQuiz(bool isIntensive) {
    if (wordList.empty()) {
        cout << "단어장이 비어있습니다. 먼저 불러오기(4번)를 해주세요." << endl;
        return;
    }

    vector<Word*> targetWords;
    for (auto& w : wordList) {
        if (!isIntensive || w.level < 2) {
            targetWords.push_back(&w);
        }
    }

    if (targetWords.empty()) {
        cout << "모든 단어를 외웠습니다! 출제할 단어가 없습니다." << endl;
        return;
    }

    random_device rd;
    mt19937 g(rd());
    shuffle(targetWords.begin(), targetWords.end(), g);

    // 일반 퀴즈는 20개, 집중 퀴즈는 모르는 거 전부!
    int maxQuestions = isIntensive ? (int)targetWords.size() : min((int)targetWords.size(), 20);
    int count = 0;
    int score = 0;

    if (isIntensive) cout << "\n--- 🔥 집중 퀴즈 시작 ---" << endl;
    else cout << "\n--- 📝 일반 퀴즈 시작  ---" << endl;

    for (Word* w : targetWords) {
        if (count >= maxQuestions) break;

        count++;
        string answer;
        cout << "\nQ" << count << ". [" << w->korean << "] 의 영단어는? : ";
        cin >> answer;

        if (answer == w->english) {
            cout << "✅ 정답!" << endl;
            score++;
            if (w->level < 2) w->level++;
        }
        else {
            cout << "❌ 오답! 정답은 [" << w->english << "] 입니다." << endl;
            w->level = 0;
            w->failCount++;
        }
    }

    cout << "\n=================================" << endl;
    cout << " 🎉 결과: " << score << " / " << count << endl;
    cout << "=================================" << endl;

    cout << "\n메인 메뉴로 돌아가려면 아무 키나 누르세요..." << endl;
    system("pause > nul");
}