#include "VocaManager.h"
#include <iostream>
#include <algorithm> 
#include <random>    
#include <iomanip> 
#include <ctime> 
#include <chrono>

using namespace std;

// =====================================================================
// [정렬 함수] UTF-8 환경에서 한글과 영문 너비를 더 정확히 계산하여 간격을 맞춤
// =====================================================================
string formatLeft(const string& str, int targetWidth) {
    int displayWidth = 0;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        if ((c & 0x80) == 0) { // 1바이트 문자 (영어, 숫자, 공백 등)
            displayWidth += 1;
            i += 1;
        }
        else if ((c & 0xE0) == 0xC0) { // 2바이트 문자
            displayWidth += 2;
            i += 2;
        }
        else if ((c & 0xF0) == 0xE0) { // 3바이트 문자 (UTF-8 한글) - 콘솔에선 보통 2칸 차지
            displayWidth += 2;
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

// [DB 초기화] 프로그램 시작 시 DB를 연결하고 단어를 불러옵니다.
bool VocaManager::init() {
    if (db.connect()) {
        wordList = db.loadAllWords(); // DB에서 전체 단어 로드
        return true;
    }
    return false;
}

// [단어 추가] 입력받은 단어를 DB에 즉시 추가합니다.
void VocaManager::addWord() {
    string eng, kor;
    cout << "\n[단어 수동 추가]" << endl;
    cout << "영어: ";
    cin >> eng;

    // 중복 검사 (메모리에 로드된 리스트 기준)
    for (const auto& w : wordList) {
        if (w.english == eng) {
            cout << "⚠️ 이미 존재하는 단어입니다! (현재 뜻: " << w.korean << ")" << endl;
            return;
        }
    }

    cin.ignore();
    cout << "한글 (여러 뜻은 쉼표로 구분해서 작성): ";
    getline(cin, kor);

    // DB에 직접 INSERT
    if (db.insertWord(eng, kor)) {
        cout << "✅ DB에 단어 추가 완료!" << endl;
        wordList = db.loadAllWords(); // 추가된 단어를 포함하여 리스트 갱신
    }
    else {
        cout << "❌ DB 저장에 실패했습니다." << endl;
    }
}

// [전체 단어 보기] - 가로 120 크기에 맞춘 표 정렬
void VocaManager::showAllWords() {
    cout << "\n==================================================================================================================================" << endl;
    cout << "                                              📚 전체 단어 목록                                              " << endl;
    cout << "==================================================================================================================================" << endl;

    cout << "  " << formatLeft("[ 영단어 ]", 25) << " | "
        << formatLeft("[ 한글 뜻 ]", 80) << " | "
        << formatLeft("[오답]", 8) << " | "
        << "[레벨]" << endl;
    cout << "----------------------------------------------------------------------------------------------------------------------------------" << endl;

    for (const auto& w : wordList) {
        cout << "  " << formatLeft(w.english, 25) << " | "
            << formatLeft(w.korean, 80) << " | "
            << formatLeft(to_string(w.failCount) + "회", 8) << " | "
            << "Lv." << w.level << endl;
    }

    cout << "==================================================================================================================================" << endl;
    cout << "총 " << wordList.size() << "개의 단어가 등록되어 있습니다." << endl;
}

// [모르는 단어 보기] - 가로 120 크기에 맞춘 표 정렬
void VocaManager::showUnknownWords() {
    long long now = time(0);
    cout << "\n==================================================================================================================================" << endl;
    cout << "                                           📖 모르는 / 복습 대기 단어                                           " << endl;
    cout << "==================================================================================================================================" << endl;

    cout << "  " << formatLeft("[ 영단어 ]", 25) << " | "
        << formatLeft("[ 한글 뜻 ]", 80) << " | "
        << formatLeft("[오답]", 8) << " | "
        << "[레벨]" << endl;
    cout << "----------------------------------------------------------------------------------------------------------------------------------" << endl;

    int reviewCount = 0;
    for (const auto& w : wordList) {
        if (w.nextReview <= now || w.level == 0) {
            cout << "  " << formatLeft(w.english, 25) << " | "
                << formatLeft(w.korean, 80) << " | "
                << formatLeft(to_string(w.failCount) + "회", 8) << " | "
                << "Lv." << w.level << endl;
            reviewCount++;
        }
    }

    if (reviewCount == 0) {
        cout << "✨ 현재 복습할 단어가 없습니다! 타이밍에 맞춰 다시 방문해주세요. ✨" << endl;
    }
    else {
        cout << "----------------------------------------------------------------------------------------------------------------------------------" << endl;
        cout << "🔔 현재 복습이 필요한 단어: " << reviewCount << "개" << endl;
    }

    cout << "\n[ 🧠 레벨별 복습 주기 (Spaced Repetition) 안내 ]" << endl;
    cout << " Lv.0 : 미학습 (즉시 출제) | Lv.1 : 1분 후      | Lv.2 : 10분 후" << endl;
    cout << " Lv.3 : 1일 후             | Lv.4 : 3일 후      | Lv.5 : 7일 후 (완료)" << endl;
    cout << " ※ 주의: 퀴즈에서 틀릴 경우 해당 단어의 레벨은 즉시 0으로 초기화됩니다." << endl;
    cout << "==================================================================================================================================\n" << endl;
}

// [퀴즈 시스템]
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

        // 1. 객관식
        if (mode == 2) {
            vector<string> options;
            options.push_back(w->english);
            vector<Word> tempDict = wordList;
            shuffle(tempDict.begin(), tempDict.end(), g);
            for (auto& tw : tempDict) {
                if (tw.english != w->english) {
                    options.push_back(tw.english);
                    if (options.size() == 4) break;
                }
            }
            shuffle(options.begin(), options.end(), g);

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
        // 2. 스크램블
        else if (mode == 3) {
            string scrambled = w->english;
            if (scrambled.length() > 1) {
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
        // 3. 타임어택
        else if (mode == 4) {
            cout << "\n[⏱️ 5초 제한!] Q" << count << ". [" << w->korean << "] 의 영단어는? : ";
            auto start = chrono::steady_clock::now();
            string answer; cin >> answer;
            auto end = chrono::steady_clock::now();
            auto diff = chrono::duration_cast<chrono::seconds>(end - start).count();

            if (answer == w->english) {
                if (diff <= 5) isCorrect = true;
                else {
                    cout << "⏰ 앗! 철자는 맞았지만 " << diff << "초가 걸렸네요. (제한시간 5초 초과)" << endl;
                    isCorrect = false;
                }
            }
            else cout << "❌ 오답! 정답은 [" << w->english << "] 입니다." << endl;
        }
        // 4. 주관식
        else {
            cout << "\nQ" << count << ". [" << w->korean << "] 의 영단어는? : ";
            string answer; cin >> answer;
            if (answer == w->english) isCorrect = true;
            else cout << "❌ 오답! 정답은 [" << w->english << "] 입니다." << endl;
        }

        // --- 에빙하우스 레벨 처리 ---
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

        // ✨ [핵심] 한 문제를 풀 때마다 변경된 레벨/오답수/다음복습시간을 DB에 바로 저장!
        db.updateWordStats(*w);
    }

    double accuracy = (count > 0) ? ((double)score / count) * 100.0 : 0.0;
    cout << "\n=================================" << endl;
    cout << " 🎉 결과: " << score << " / " << count << " 맞춤" << endl;
    cout << " 🎯 정답률: " << fixed << setprecision(1) << accuracy << "%" << endl;
    cout << "=================================" << endl;

    cout << "\n메인 메뉴로 돌아가려면 아무 키나 누르세요..." << endl;
    system("pause > nul");
}