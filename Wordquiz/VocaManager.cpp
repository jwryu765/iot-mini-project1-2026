#include "VocaManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm> 
#include <random>    
#include <iomanip> 
#include <ctime> 
#include <chrono> // [추가됨] 타임어택 시간 측정을 위한 헤더

using namespace std;

// =====================================================================
// [핵심 추가] 한글과 영문의 콘솔 출력 너비를 정확히 계산하여 공백을 채우는 함수
// =====================================================================
string formatLeft(const string& str, int targetWidth) {
    int displayWidth = 0;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        if (c < 0x80) { // 1바이트 문자 (알파벳, 숫자, 공백)
            displayWidth += 1;
            i += 1;
        }
        else if ((c & 0xE0) == 0xC0) { // 2바이트 문자
            displayWidth += 2;
            i += 2;
        }
        else if ((c & 0xF0) == 0xE0) { // 3바이트 문자 (한글)
            displayWidth += 2; // 한글은 콘솔에서 시각적으로 2칸을 차지함
            i += 3;
        }
        else if ((c & 0xF8) == 0xF0) { // 4바이트 문자
            displayWidth += 2;
            i += 4;
        }
        else {
            displayWidth += 1;
            i += 1;
        }
    }

    int padding = targetWidth - displayWidth;
    if (padding < 0) padding = 0;
    return str + string(padding, ' ');
}
// =====================================================================

// [강력해진 CSV 파서] 쉼표, 따옴표, 줄바꿈 방어 + 복습 시간 파싱
void VocaManager::loadFromCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "voca.csv 파일이 없습니다. 새로 시작합니다!" << endl;
        system("pause > nul");
        return;
    }

    string line;
    string fullLine = "";
    wordList.clear();

    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (fullLine.empty()) {
            fullLine = line;
        }
        else {
            fullLine += " " + line;
        }

        int quoteCount = 0;
        for (char c : fullLine) {
            if (c == '"') quoteCount++;
        }
        if (quoteCount % 2 != 0) {
            continue;
        }

        vector<string> tokens;
        string currentToken = "";
        bool insideQuotes = false;

        for (size_t i = 0; i < fullLine.length(); ++i) {
            char c = fullLine[i];
            if (c == '"') {
                if (insideQuotes && i + 1 < fullLine.length() && fullLine[i + 1] == '"') {
                    currentToken += '"';
                    i++;
                }
                else {
                    insideQuotes = !insideQuotes;
                }
            }
            else if (c == ',' && !insideQuotes) {
                tokens.push_back(currentToken);
                currentToken = "";
            }
            else {
                currentToken += c;
            }
        }
        tokens.push_back(currentToken);

        if (tokens.size() >= 2) {
            string eng = tokens[0];
            string kor = tokens[1];
            int levelVal = 0, failVal = 0;
            long long nextRevVal = 0;

            if (tokens.size() > 2 && !tokens[2].empty()) {
                try { levelVal = stoi(tokens[2]); }
                catch (...) {}
            }
            if (tokens.size() > 3 && !tokens[3].empty()) {
                try { failVal = stoi(tokens[3]); }
                catch (...) {}
            }
            if (tokens.size() > 4 && !tokens[4].empty()) {
                try { nextRevVal = stoll(tokens[4]); }
                catch (...) {}
            }

            wordList.push_back(Word(eng, kor, levelVal, failVal, nextRevVal));
        }

        fullLine = "";
    }
    file.close();
}

void VocaManager::saveToCSV(const string& filename) {
    ofstream file(filename);
    for (const auto& w : wordList) {
        string safeKor = "";
        for (char c : w.korean) {
            if (c == '"') safeKor += "\"\"";
            else safeKor += c;
        }
        file << w.english << ",\"" << safeKor << "\"," << w.level << "," << w.failCount << "," << w.nextReview << "\n";
    }
    file.close();
}

void VocaManager::addWord() {
    string eng, kor;
    cout << "\n[단어 수동 추가]" << endl;
    cout << "영어: ";
    cin >> eng;

    for (const auto& w : wordList) {
        if (w.english == eng) {
            cout << "⚠️ 이미 존재하는 단어입니다! (현재 뜻: " << w.korean << ")" << endl;
            return;
        }
    }

    cin.ignore();
    cout << "한글 (여러 뜻은 쉼표로 구분해서 작성): ";
    getline(cin, kor);

    wordList.push_back(Word(eng, kor));

    saveToCSV("voca.csv");
    cout << "✅ 추가 및 저장 완료!" << endl;
}

void VocaManager::showAllWords() {
    cout << "\n===========================================================================================================================" << endl;
    cout << "                                                    📚 전체 단어 목록                                                    " << endl;
    cout << "===========================================================================================================================" << endl;

    // ✨ [수정됨] 간격 6으로 수정 및 통일
    cout << "  " << formatLeft("[ 영단어 ]", 20) << " | "
        << formatLeft("[ 한글 뜻 ]", 80) << " | "
        << formatLeft("[오답]", 6) << " | "
        << "[레벨]" << endl;
    cout << "---------------------------------------------------------------------------------------------------------------------------" << endl;

    for (const auto& w : wordList) {
        cout << "  " << formatLeft(w.english, 20) << " | "
            << formatLeft(w.korean, 80) << " | "
            << formatLeft(to_string(w.failCount) + "회", 6) << " | "
            << "Lv." << w.level << endl;
    }

    cout << "===========================================================================================================================\n" << endl;
    cout << "총 " << wordList.size() << "개의 단어가 등록되어 있습니다." << endl;
}

void VocaManager::showUnknownWords() {
    long long now = time(0);
    cout << "\n=========================================================================================================================" << endl;
    cout << "                                                 📖 모르는 / 복습 대기 단어                                                " << endl;
    cout << "=========================================================================================================================" << endl;

    // ✨ [수정됨] 간격 6으로 수정 및 showAllWords와 동일하게 양식 통일
    cout << "  " << formatLeft("[ 영단어 ]", 20) << " | "
        << formatLeft("[ 한글 뜻 ]", 80) << " | "
        << formatLeft("[오답]", 6) << " | "
        << "[레벨]" << endl;
    cout << "-------------------------------------------------------------------------------------------------------------------------" << endl;

    int reviewCount = 0;
    for (const auto& w : wordList) {
        if (w.nextReview <= now || w.level == 0) {
            cout << "  " << formatLeft(w.english, 20) << " | "
                << formatLeft(w.korean, 80) << " | "
                << formatLeft(to_string(w.failCount) + "회", 6) << " | "
                << "Lv." << w.level << endl;
            reviewCount++;
        }
    }

    if (reviewCount == 0) {
        cout << "✨ 현재 복습할 단어가 없습니다! 타이밍에 맞춰 다시 방문해주세요. ✨" << endl;
    }
    else {
        cout << "---------------------------------------------------------------------------------------------------------------------------" << endl;
        cout << "🔔 현재 복습이 필요한 단어: " << reviewCount << "개" << endl;
    }

    cout << "\n[ 🧠 레벨별 복습 주기 (Spaced Repetition) 안내 ]" << endl;
    cout << " Lv.0 : 미학습 (즉시 출제) | Lv.1 : 1분 후      | Lv.2 : 10분 후" << endl;
    cout << " Lv.3 : 1일 후             | Lv.4 : 3일 후      | Lv.5 : 7일 후 (완료)" << endl;
    cout << " ※ 주의: 퀴즈에서 틀릴 경우 해당 단어의 레벨은 즉시 0으로 초기화됩니다." << endl;
    cout << "===========================================================================================================================\n" << endl;
}

// [기능 추가] 🎮 게임 모드가 추가된 퀴즈 시스템!
void VocaManager::runQuiz(bool isIntensive) {
    if (wordList.empty()) {
        cout << "단어장이 비어있습니다. 단어를 먼저 추가해주세요." << endl;
        system("pause > nul");
        return;
    }

    long long now = time(0);
    vector<Word*> targetWords;

    for (auto& w : wordList) {
        if (isIntensive) {
            if (w.failCount > 0 || w.level == 0) targetWords.push_back(&w);
        }
        else {
            if (w.nextReview <= now) targetWords.push_back(&w);
        }
    }

    if (targetWords.empty()) {
        if (!isIntensive) cout << "😎 현재 복습 주기가 돌아온 단어가 없습니다!" << endl;
        else cout << "🎉 오답 단어가 없습니다! 완벽합니다." << endl;
        system("pause > nul");
        return;
    }

    // 퀴즈 모드 선택 메뉴 출력
    int mode = 1;
    cout << "\n=================================" << endl;
    cout << "         🎮 퀴즈 모드 선택       " << endl;
    cout << "=================================" << endl;
    cout << "1. 📝 주관식 (기본)" << endl;
    cout << "2. 🔢 객관식 (4지선다)" << endl;
    cout << "3. 🔀 스크램블 (철자 맞추기)" << endl;
    cout << "4. ⏱️ 타임어택 (5초 제한 주관식)" << endl;
    cout << "=================================" << endl;
    cout << "모드 번호를 입력하세요: ";
    cin >> mode;

    // 객관식은 최소 단어 4개가 필요하므로 예외 처리
    if (mode == 2 && wordList.size() < 4) {
        cout << "\n⚠️ 객관식 모드를 하려면 전체 단어장에 최소 4개의 단어가 필요합니다." << endl;
        cout << "기본 주관식 모드로 강제 전환됩니다." << endl;
        mode = 1;
    }

    random_device rd;
    mt19937 g(rd());
    shuffle(targetWords.begin(), targetWords.end(), g);

    int maxQuestions = isIntensive ? (int)targetWords.size() : min((int)targetWords.size(), 20);
    int count = 0;
    int score = 0;

    if (isIntensive) cout << "\n--- 🔥 오답 집중 훈련 시작 ---" << endl;
    else cout << "\n--- 📝 복습 퀴즈 시작 ---" << endl;

    for (Word* w : targetWords) {
        if (count >= maxQuestions) break;

        count++;
        bool isCorrect = false;

        // ==========================================
        // 1. 객관식 모드 (4지선다)
        // ==========================================
        if (mode == 2) {
            vector<string> options;
            options.push_back(w->english); // 정답 넣기

            // 전체 단어장에서 정답이 아닌 단어 3개 랜덤 추출
            vector<Word> tempDict = wordList;
            shuffle(tempDict.begin(), tempDict.end(), g);
            for (auto& tw : tempDict) {
                if (tw.english != w->english) {
                    options.push_back(tw.english);
                    if (options.size() == 4) break;
                }
            }
            shuffle(options.begin(), options.end(), g); // 보기 4개 섞기

            cout << "\nQ" << count << ". [" << w->korean << "] 의 영단어는?" << endl;
            int correctNum = 0, ansNum = 0;
            for (int i = 0; i < 4; i++) {
                cout << "  " << i + 1 << ") " << options[i] << endl;
                if (options[i] == w->english) correctNum = i + 1;
            }
            cout << "정답 번호: ";
            cin >> ansNum;

            if (ansNum == correctNum) isCorrect = true;
            else cout << "❌ 오답! 정답은 [" << w->english << "] 입니다." << endl;
        }
        // ==========================================
        // 2. 스크램블 모드 (철자 맞추기)
        // ==========================================
        else if (mode == 3) {
            string scrambled = w->english;
            if (scrambled.length() > 1) {
                // 원래 단어와 무조건 다르게 섞일 때까지 반복
                while (scrambled == w->english) {
                    shuffle(scrambled.begin(), scrambled.end(), g);
                }
            }
            cout << "\nQ" << count << ". 철자를 조합해 단어를 맞추세요!" << endl;
            cout << "👉 [ " << scrambled << " ] (뜻: " << w->korean << ")" << endl;
            cout << "정답 입력: ";
            string answer; cin >> answer;

            if (answer == w->english) isCorrect = true;
            else cout << "❌ 오답! 정답은 [" << w->english << "] 입니다." << endl;
        }
        // ==========================================
        // 3. 타임어택 모드 (5초 제한)
        // ==========================================
        else if (mode == 4) {
            cout << "\n[⏱️ 5초 제한!] Q" << count << ". [" << w->korean << "] 의 영단어는? : ";

            // 시간 측정 시작
            auto start = chrono::steady_clock::now();
            string answer; cin >> answer;
            // 시간 측정 종료
            auto end = chrono::steady_clock::now();
            auto diff = chrono::duration_cast<chrono::seconds>(end - start).count();

            if (answer == w->english) {
                if (diff <= 5) {
                    isCorrect = true;
                }
                else {
                    cout << "⏰ 앗! 철자는 맞았지만 " << diff << "초가 걸렸네요. (제한시간 5초 초과)" << endl;
                    isCorrect = false;
                }
            }
            else cout << "❌ 오답! 정답은 [" << w->english << "] 입니다." << endl;
        }
        // ==========================================
        // 4. 일반 주관식 모드 (기본)
        // ==========================================
        else {
            cout << "\nQ" << count << ". [" << w->korean << "] 의 영단어는? : ";
            string answer; cin >> answer;
            if (answer == w->english) isCorrect = true;
            else cout << "❌ 오답! 정답은 [" << w->english << "] 입니다." << endl;
        }

        // --- 공통: 정답/오답에 따른 에빙하우스 레벨 처리 ---
        if (isCorrect) {
            cout << "✅ 정답!" << endl;
            score++;
            if (w->level < 5) w->level++;
            int intervals[] = { 0, 1, 10, 1440, 4320, 10080 };
            w->nextReview = time(0) + (intervals[w->level] * 60);
            cout << "📈 레벨 UP! (현재 레벨: " << w->level << ")" << endl;
        }
        else {
            w->level = 0;
            w->failCount++;
            w->nextReview = time(0);
        }
    }

    double accuracy = (count > 0) ? ((double)score / count) * 100.0 : 0.0;
    cout << "\n=================================" << endl;
    cout << " 🎉 결과: " << score << " / " << count << " 맞춤" << endl;
    cout << " 🎯 정답률: " << fixed << setprecision(1) << accuracy << "%" << endl;
    cout << "=================================" << endl;

    saveToCSV("voca.csv");
    cout << "\n메인 메뉴로 돌아가려면 아무 키나 누르세요..." << endl;
    system("pause > nul");
}