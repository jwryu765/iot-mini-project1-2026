#include "VocaManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm> 
#include <random>    
#include <iomanip> // 표 출력을 위한 헤더 추가

using namespace std;

// [수정됨] 엑셀 줄바꿈(Alt+Enter) 처리 및 자동 불러오기
void VocaManager::loadFromCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "voca.csv 파일이 없습니다. 새로 시작합니다!" << endl;
        system("pause > nul");
        return;
    }

    string line;
    string fullLine = "";
    bool inQuotes = false;
    wordList.clear();

    while (getline(file, line)) {
        if (line.empty() && !inQuotes) continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();

        // 쌍따옴표 개수를 세서 현재 줄바꿈이 따옴표 내부인지 확인
        for (char c : line) {
            if (c == '"') inQuotes = !inQuotes;
        }

        if (fullLine.empty()) {
            fullLine = line;
        }
        else {
            // 따옴표 내부에서 줄바꿈이 일어난 경우 공백으로 묶어서 한 줄로 합침
            fullLine += " " + line;
        }

        if (inQuotes) {
            continue; // 따옴표가 아직 안 닫혔으면 다음 줄을 마저 읽어옴
        }

        // --- 여기서부터는 완전한 한 줄(fullLine)을 분석 ---
        size_t lastComma = fullLine.find_last_of(',');
        size_t secondLastComma = (lastComma != string::npos) ? fullLine.find_last_of(',', lastComma - 1) : string::npos;
        size_t firstComma = fullLine.find_first_of(',');

        if (firstComma == string::npos || secondLastComma == string::npos || firstComma >= secondLastComma) {
            fullLine = ""; // 형식 오류시 초기화 후 건너뜀
            continue;
        }

        string eng = fullLine.substr(0, firstComma);
        string kor = fullLine.substr(firstComma + 1, secondLastComma - firstComma - 1);

        // 엑셀에서 줄바꿈 시 자동으로 붙이는 양끝 큰따옴표(" ") 제거
        if (kor.length() >= 2 && kor.front() == '"' && kor.back() == '"') {
            kor = kor.substr(1, kor.length() - 2);
        }

        string lvlStr = fullLine.substr(secondLastComma + 1, lastComma - secondLastComma - 1);
        string failStr = fullLine.substr(lastComma + 1);

        try {
            wordList.push_back(Word(eng, kor, stoi(lvlStr), stoi(failStr)));
        }
        catch (...) {
            wordList.push_back(Word(eng, kor, 0, 0));
        }

        fullLine = ""; // 다음 단어를 위해 초기화
    }
    file.close();
}

// CSV 저장 기능 탑재 (기존 데이터와 병합하여 최신 상태 저장)
void VocaManager::saveToCSV(const string& filename) {
    ofstream file(filename);
    for (const auto& w : wordList) {
        file << w.english << "," << w.korean << "," << w.level << "," << w.failCount << "\n";
    }
    file.close();
}

void VocaManager::addWord() {
    string eng, kor;
    cout << "\n[단어 수동 추가]" << endl;
    cout << "영어: ";
    cin >> eng;

    // 중복 단어 체크 로직
    for (const auto& w : wordList) {
        if (w.english == eng) {
            cout << "⚠️ 이미 존재하는 단어입니다! (현재 뜻: " << w.korean << ")" << endl;
            return;
        }
    }

    cin.ignore(); // 버퍼 비우기
    cout << "한글 (여러 뜻은 쉼표로 구분해서 작성): ";
    getline(cin, kor); // 띄어쓰기나 쉼표를 포함하여 입력받기

    wordList.push_back(Word(eng, kor));

    // 단어 추가 후 즉시 저장하여 기존 단어장과 병합
    saveToCSV("voca.csv");
    cout << "✅ 추가 및 저장 완료!" << endl;
}

// 전체 단어장 보기 (표 형태 출력 및 쉼표 포함 뜻 출력)
void VocaManager::showAllWords() {
    cout << "\n=======================================================" << endl;
    cout << "                    📚 전체 단어장                     " << endl;
    cout << "=======================================================" << endl;

    // 표 헤더 생성 (영단어 공간 20칸 확보)
    cout << left << setw(20) << "  [ 영단어 ]" << " | " << "[ 한글 뜻 ]" << endl;
    cout << "-------------------------------------------------------" << endl;

    for (const auto& w : wordList) {
        cout << "  " << left << setw(18) << w.english << " | " << w.korean << endl;
    }

    cout << "=======================================================\n" << endl;
    cout << "총 " << wordList.size() << "개의 단어가 있습니다." << endl;
}

void VocaManager::showUnknownWords() {
    cout << "\n=======================================================" << endl;
    cout << "                   📖 모르는 단어장                    " << endl;
    cout << "=======================================================" << endl;

    int count = 0;
    for (const auto& w : wordList) {
        if (w.level < 2) {
            cout << "- " << left << setw(18) << w.english << " : " << w.korean
                << " (오답: " << w.failCount << "회)" << endl;
            count++;
        }
    }

    if (count == 0) {
        cout << "축하합니다! 모든 단어를 마스터하셨습니다!" << endl;
    }
    else {
        cout << "\n현재 복습이 필요한 단어: " << count << "개" << endl;
    }
    cout << "=======================================================\n" << endl;
}

void VocaManager::runQuiz(bool isIntensive) {
    if (wordList.empty()) {
        cout << "단어장이 비어있습니다. 단어를 먼저 추가해주세요." << endl;
        system("pause > nul");
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
        system("pause > nul");
        return;
    }

    random_device rd;
    mt19937 g(rd());
    shuffle(targetWords.begin(), targetWords.end(), g);

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

    // 정답률 계산 및 표시 로직
    double accuracy = (count > 0) ? ((double)score / count) * 100.0 : 0.0;

    cout << "\n=================================" << endl;
    cout << " 🎉 결과: " << score << " / " << count << " 맞춤" << endl;
    cout << " 🎯 정답률: " << fixed << setprecision(1) << accuracy << "%" << endl;
    cout << "=================================" << endl;

    cout << "\n메인 메뉴로 돌아가려면 아무 키나 누르세요..." << endl;
    system("pause > nul");
}