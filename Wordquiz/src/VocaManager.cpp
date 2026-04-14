#include "VocaManager.h"
#include <iostream>
#include <algorithm> 
#include <random>    
#include <iomanip> 
#include <ctime> 
#include <chrono>
#include <string>
#include <sstream> 
#include <limits> 
#include <cmath>  
#include <cctype>
#include <windows.h> 
#include <fstream>   
#include <conio.h>   

using namespace std;

bool isValidEnglish(const string& str) {
    if (str.empty() || str == "0") return true;
    bool hasAlphabet = false;
    for (char c : str) {
        if ((c & 0x80) != 0) return false;
        if (isalpha((unsigned char)c)) hasAlphabet = true;
        else if (!isspace((unsigned char)c) && c != '-' && c != '\'' && !isdigit((unsigned char)c)) return false;
    }
    return hasAlphabet;
}

bool isValidKorean(const string& str) {
    if (str.empty() || str == "0") return true;
    for (char c : str) {
        if ((c & 0x80) != 0) return true;
    }
    return false;
}

string getSafeString(const string& prompt) {
    string inputStr;
    while (true) {
        cout << prompt;
        getline(cin, inputStr);
        if (inputStr.empty() || inputStr.find_first_not_of(" \t") == string::npos) {
            cout << "\x1b[1A\x1b[2K";
            continue;
        }
        return inputStr;
    }
}

int getSafeInputInManager(const string& prompt) {
    string inputStr;
    int inputNum;
    while (true) {
        cout << prompt;
        getline(cin, inputStr);
        if (inputStr.empty() || inputStr.find_first_not_of(" \t") == string::npos) {
            cout << "\x1b[1A\x1b[2K";
            continue;
        }
        stringstream ss(inputStr);
        if (ss >> inputNum && ss.eof()) return inputNum;
        else {
            cout << "⚠️ 숫자를 입력해주세요!" << endl;
            system("pause > nul");
            cout << "\x1b[1A\x1b[2K\x1b[1A\x1b[2K";
        }
    }
}

string formatLeft(const string& str, int targetWidth) {
    int displayWidth = 0;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        if ((c & 0x80) == 0) { displayWidth += 1; i += 1; }
        else if ((c & 0xE0) == 0xC0) { displayWidth += 2; i += 2; }
        else if ((c & 0xF0) == 0xE0) { displayWidth += 2; i += 3; }
        else if ((c & 0xF8) == 0xF0) { displayWidth += 2; i += 4; }
        else { displayWidth += 1; i += 1; }
    }
    int padding = targetWidth - displayWidth;
    if (padding < 0) padding = 0;
    return str + string(padding, ' ');
}

bool VocaManager::init() {
    if (db.connect()) return true;
    return false;
}

void VocaManager::addWord() {
    cout << "\n=================================" << endl;
    cout << "         ➕ 단어 수동 추가       " << endl;
    cout << "=================================" << endl;
    cout << "💡 (취소하고 메인 메뉴로 돌아가려면 '0'을 입력하세요)\n" << endl;

    string eng, kor;

    while (true) {
        eng = getSafeString("영어: ");
        if (eng == "0") return;
        if (isValidEnglish(eng)) break;
        cout << "⚠️ [경고] 영단어 칸에는 한글이나 특수기호를 쓸 수 없습니다!\n" << endl;
    }

    for (const auto& w : wordList) {
        if (w.english == eng) {
            cout << "⚠️ 이미 존재하는 단어입니다! (현재 뜻: " << w.korean << ")" << endl;
            system("pause > nul");
            return;
        }
    }

    while (true) {
        kor = getSafeString("한글 (여러 뜻은 쉼표로 구분): ");
        if (kor == "0") return;
        if (isValidKorean(kor)) break;
        cout << "⚠️ [경고] 뜻에는 반드시 한글이 포함되어야 합니다!\n" << endl;
    }

    if (db.insertWord(eng, kor)) {
        cout << "✅ DB에 단어 추가 완료! (" << eng << " : " << kor << ")" << endl;
        wordList = db.loadAllWords(currentUser);
    }
    else cout << "❌ DB 저장 실패." << endl;
}

void VocaManager::loadFromCSV() {
    cout << "\n=================================" << endl;
    cout << "     📂 CSV 파일로 단어 추가     " << endl;
    cout << "=================================" << endl;
    cout << "💡 프로젝트 폴더에 'voca_list.csv' 파일을 넣어주세요." << endl;
    cout << "💡 형식 예시: apple,사과 (띄어쓰기 없이 쉼표로만 구분)" << endl;

    string filename = "voca_list.csv";
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "\n❌ '" << filename << "' 파일을 찾을 수 없습니다." << endl;
        system("pause > nul");
        return;
    }

    string line;
    int successCount = 0;
    int failCount = 0;

    cout << "\n데이터를 분석하고 DB에 저장하는 중..." << endl;

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string eng, kor, extra;

        if (getline(ss, eng, ',')) {
            if (getline(ss, kor, ',')) {
            }
            else {
                ss.clear();
                ss.str(line);
                getline(ss, eng, ',');
                getline(ss, kor);
            }

            auto cleanData = [](string& s) {
                s.erase(0, s.find_first_not_of(" \t\r\n"));
                if (!s.empty()) s.erase(s.find_last_not_of(" \t\r\n") + 1);
                if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
                    s = s.substr(1, s.size() - 2);
                }
                s.erase(0, s.find_first_not_of(" \t\r\n"));
                if (!s.empty()) s.erase(s.find_last_not_of(" \t\r\n") + 1);
                };

            cleanData(eng);
            cleanData(kor);

            if (!eng.empty() && !kor.empty()) {
                if (db.insertWord(eng, kor)) {
                    successCount++;
                }
                else {
                    failCount++;
                }
            }
        }
    }

    file.close();
    wordList = db.loadAllWords(currentUser);

    cout << "\n✅ 처리가 완료되었습니다!" << endl;
    cout << "   - 성공적으로 추가된 단어: " << successCount << "건" << endl;
    cout << "   - 실패 (중복된 단어 등): " << failCount << "건" << endl;
    cout << "=================================" << endl;
    cout << "메인 메뉴로 돌아가려면 아무 키나 누르세요...";
    system("pause > nul");
}

void VocaManager::showAllWords() {
    string keyword = "";

    while (true) {
        system("cls");
        cout << "\n==================================================================================================================================" << endl;
        cout << "                                                    📚 전체 단어 목록                                                            " << endl;
        cout << "==================================================================================================================================" << endl;
        cout << "  " << formatLeft("[ 영단어 ]", 25) << " | " << formatLeft("[ 한글 뜻 ]", 80) << " | " << formatLeft("[오답]", 8) << " | [레벨]" << endl;
        cout << "----------------------------------------------------------------------------------------------------------------------------------" << endl;

        int displayCount = 0;
        for (const auto& w : wordList) {
            if (keyword.empty() || w.english.find(keyword) != string::npos || w.korean.find(keyword) != string::npos) {
                cout << "  " << formatLeft(w.english, 25) << " | " << formatLeft(w.korean, 80) << " | " << formatLeft(to_string(w.failCount) + "회", 8) << " | Lv." << w.level << endl;
                displayCount++;
            }
        }
        cout << "==================================================================================================================================" << endl;

        if (keyword.empty()) {
            cout << "총 " << displayCount << "개의 단어가 등록되어 있습니다." << endl;
        }
        else {
            cout << "🔍 '" << keyword << "' 검색 결과: 총 " << displayCount << "개 발견" << endl;
        }

        int padLines = 20 - displayCount;
        if (padLines > 0) {
            for (int i = 0; i < padLines; i++) cout << "\n";
        }

        cout << "\n▶ 검색할 단어/뜻을 입력하세요 (초기화: 1 / 뒤로가기: 0) : ";
        string input;
        getline(cin, input);

        if (input == "0") break;
        else if (input == "1") keyword = "";
        else if (input.empty() || input.find_first_not_of(" \t") == string::npos) continue;
        else keyword = input;
    }
}

void VocaManager::showReviewList() {
    long long now = time(0);
    string keyword = "";

    while (true) {
        system("cls");
        cout << "\n==================================================================================================================================" << endl;
        cout << "                                              🎯 오답 단어 몰아보기                                                              " << endl;
        cout << "==================================================================================================================================" << endl;
        cout << "  " << formatLeft("[ 영단어 ]", 25) << " | " << formatLeft("[ 한글 뜻 ]", 80) << " | " << formatLeft("[오답]", 8) << " | [레벨]" << endl;
        cout << "----------------------------------------------------------------------------------------------------------------------------------" << endl;

        int totalReviewCount = 0;
        int displayCount = 0;

        for (const auto& w : wordList) {
            if (w.nextReview <= now || w.level == 0) {
                totalReviewCount++;
                if (keyword.empty() || w.english.find(keyword) != string::npos || w.korean.find(keyword) != string::npos) {
                    cout << "  " << formatLeft(w.english, 25) << " | " << formatLeft(w.korean, 80) << " | " << formatLeft(to_string(w.failCount) + "회", 8) << " | Lv." << w.level << endl;
                    displayCount++;
                }
            }
        }

        if (totalReviewCount == 0) {
            cout << "✨ 현재 복습할 단어가 없습니다! ✨" << endl;
            cout << "==================================================================================================================================\n" << endl;
            system("pause > nul");
            return;
        }

        cout << "==================================================================================================================================\n" << endl;

        if (keyword.empty()) {
            cout << "🔔 현재 복습이 필요한 단어: " << displayCount << "개" << endl;
        }
        else {
            cout << "🔍 '" << keyword << "' 검색 결과: " << displayCount << "개 발견 (전체 복습 대상: " << totalReviewCount << "개)" << endl;
        }

        int padLines = 20 - displayCount;
        if (padLines > 0) {
            for (int i = 0; i < padLines; i++) cout << "\n";
        }

        cout << "\n▶ 검색할 단어/뜻을 입력하세요 (초기화: 1 / 뒤로가기: 0) : ";
        string input;
        getline(cin, input);

        if (input == "0") break;
        else if (input == "1") keyword = "";
        else if (input.empty() || input.find_first_not_of(" \t") == string::npos) continue;
        else keyword = input;
    }
}

void VocaManager::runQuiz(bool isIntensive) {
    if (wordList.empty()) {
        cout << "단어장이 비어있습니다." << endl;
        system("pause > nul");
        return;
    }

    long long now = time(0);
    vector<Word*> targetWords;

    for (auto& w : wordList) {
        if (isIntensive) {
            if (w.failCount > 0) targetWords.push_back(&w);
        }
        else {
            if (w.nextReview <= now || w.level == 0) targetWords.push_back(&w);
        }
    }

    if (isIntensive && targetWords.size() < 20) {
        system("cls");
        cout << "\n==========================================================================" << endl;
        cout << "                       🚫 집중 퀴즈 입장 불가 🚫                          " << endl;
        cout << "==========================================================================\n" << endl;
        cout << "  ⚠️ 집중 퀴즈는 '진짜 틀린 오답'이 최소 '20개 이상' 모였을 때만 도전할 수 있습니다." << endl;
        cout << "  (현재 순수 오답 단어: " << targetWords.size() << "개 / 목표: 20개)\n" << endl;
        cout << "  일반 퀴즈를 통해 단어를 더 학습하고 오답을 모은 뒤 다시 찾아주세요!" << endl;
        cout << "\n==========================================================================" << endl;
        cout << "메인 메뉴로 돌아가려면 아무 키나 누르세요...";
        system("pause > nul");
        return;
    }

    if (targetWords.empty()) {
        cout << "😎 지금 풀 수 있는 퀴즈가 없습니다!" << endl;
        system("pause > nul");
        return;
    }

    cout << "\n=================================" << endl;
    cout << "         🎮 퀴즈 모드 선택       " << endl;
    cout << "=================================" << endl;
    cout << "1. 📝 주관식 (기본)" << endl;
    cout << "2. 🔢 객관식 (4지선다)" << endl;
    cout << "3. 🔀 스크램블 (철자 맞추기)" << endl;
    cout << "4. ⏱️ 10초 타임어택 (주관식)" << endl;
    cout << "0. 🔙 뒤로 가기 (메인 메뉴)" << endl;
    cout << "=================================" << endl;

    int mode = getSafeInputInManager("모드 번호를 입력하세요: ");
    if (mode == 0) return;

    if (mode == 2 && wordList.size() < 4) {
        cout << "\n⚠️ 단어가 4개 미만이라 주관식으로 전환됩니다." << endl;
        mode = 1;
    }

    random_device rd;
    mt19937 g(rd());
    shuffle(targetWords.begin(), targetWords.end(), g);

    int maxQuestions = isIntensive ? (int)targetWords.size() : min((int)targetWords.size(), 20);
    int count = 0;
    int score = 0;

    for (Word* w : targetWords) {
        if (count >= maxQuestions) break;
        count++;
        bool isCorrect = false;
        bool isTimeout = false;

        if (mode == 2) {
            vector<string> options = { w->english };
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
            int correctNum = 0;
            for (int i = 0; i < 4; i++) {
                cout << "  " << i + 1 << ") " << options[i] << endl;
                if (options[i] == w->english) correctNum = i + 1;
            }

            int ansNum = getSafeInputInManager("정답 번호: ");
            if (ansNum == correctNum) isCorrect = true;
            else cout << "❌ 오답! 정답은 [" << w->english << "] 입니다." << endl;
        }
        else if (mode == 3) {
            string scrambled = w->english;
            if (scrambled.length() > 1) while (scrambled == w->english) shuffle(scrambled.begin(), scrambled.end(), g);
            cout << "\nQ" << count << ". 철자를 조합하세요! 👉 [ " << scrambled << " ] (" << w->korean << ")" << endl;
            string answer = getSafeString("정답: ");
            if (answer == w->english) isCorrect = true;
            else cout << "❌ 오답! 정답은 [" << w->english << "] 입니다." << endl;
        }
        else if (mode == 4) {
            /*
             * [Non-blocking I/O 및 콘솔 렌더링 최적화]
             * 표준 입력(cin)은 스레드를 블로킹하므로 타이머와 동시 실행이 불가합니다.
             * 이를 해결하기 위해 _kbhit()과 _getch()를 활용해 비블로킹 방식으로 실시간 키 입력을 감지합니다.
             * 또한, 화면 갱신 시 system("cls")를 쓰면 심한 깜빡임(Flickering)이 생기므로,
             * ANSI Escape Code(\33[2K)와 캐리지 리턴(\r)을 결합해 현재 줄만 덮어쓰도록 렌더링을 최적화했습니다.
             */
            string answer = "";
            auto startTime = chrono::steady_clock::now();
            int limit = 10;

            int lastRemaining = -1;
            string lastAnswer = "[INIT]";

            cout << "\nQ" << count << ". [" << w->korean << "] 의 영단어는?" << endl;

            while (true) {
                auto currentTime = chrono::steady_clock::now();
                int elapsed = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();
                int remaining = limit - elapsed;

                if (remaining != lastRemaining || answer != lastAnswer) {
                    cout << "\33[2K\r";

                    if (remaining <= 3) cout << " \033[31m[⏰ " << remaining << "초]\033[0m 정답 입력: " << answer;
                    else cout << " [⏱️ " << remaining << "초] 정답 입력: " << answer;

                    lastRemaining = remaining;
                    lastAnswer = answer;
                }

                if (elapsed >= limit) {
                    isTimeout = true;
                    break;
                }

                if (_kbhit()) {
                    char ch = _getch();
                    if (ch == 13) break;
                    else if (ch == 8) {
                        if (!answer.empty()) answer.pop_back();
                    }
                    else if (ch >= 32 && ch <= 126) {
                        answer += ch;
                    }
                }
                Sleep(50);
            }

            cout << endl;

            if (isTimeout) {
                cout << "❌ 시간 초과! 너무 늦었습니다. 정답은 [" << w->english << "] 입니다." << endl;
                isCorrect = false;
            }
            else {
                if (answer == w->english) isCorrect = true;
                else cout << "❌ 오답! 정답은 [" << w->english << "] 입니다." << endl;
            }
        }
        else {
            cout << endl;
            string prompt = "Q" + to_string(count) + ". [" + w->korean + "] 의 영단어는? : ";
            string answer = getSafeString(prompt);
            if (answer == w->english) isCorrect = true;
            else cout << "❌ 오답! 정답은 [" << w->english << "] 입니다." << endl;
        }

        if (isCorrect) {
            db.addScore(currentUser, 10);
            score++;
            bool justMastered = false;

            if (w->level < 5) {
                w->level++;
                if (w->level == 5) justMastered = true;
            }
            int intervals[] = { 0, 1, 10, 1440, 4320, 10080 };
            w->nextReview = time(0) + (intervals[w->level] * 60);

            if (!justMastered) {
                cout << "✅ 정답! (현재 레벨: Lv." << w->level << ")" << endl;
                Beep(750, 100); Beep(1000, 150);
            }
            else {
                cout << R"(
    * .  * .   * .
  .   🎉 CONGRATULATIONS! 🎉   *
    .    * .     * .
)" << '\n';
                cout << " 대단합니다! [" << w->english << "] 단어를 완벽하게 마스터했습니다! (Lv.5 도달)\n" << endl;
                Beep(523, 100); Beep(659, 100); Beep(784, 100); Beep(1046, 300);
            }
        }
        else {
            w->level = 0;
            w->failCount++;
            w->nextReview = time(0);
            Beep(300, 300);
        }

        db.updateWordStats(*w, currentUser);
    }

    double accuracy = (count > 0) ? ((double)score / count) * 100.0 : 0.0;
    cout << "\n=================================" << endl;
    cout << " 🎉 퀴즈 종료! 결과: " << score << " / " << count << " 맞춤 (" << fixed << setprecision(1) << accuracy << "%)" << endl;
    cout << "=================================" << endl;
    system("pause > nul");
}

void VocaManager::showStartScreen() {
    system("cls");
    cout << R"(
==========================================================================
           __        __            _          ___        _     
           \ \      / /__  _ __ __| |        / _ \ _   _(_)____
            \ \ /\ / / _ \| '__/ _` |  ____ | | | | | | | |_  /
             \ V  V / (_) | | | (_| | |____|| |_| | |_| | |/ / 
              \_/\_/ \___/|_|  \__,_|        \__\_\\__,_|_/___|
                                                     
==========================================================================
)";
    cout << "                       [ 영단어 학습 마스터 v6.4 ]" << endl;
    cout << "==========================================================================\n" << endl;
    cout << "      오늘도 힘차게 단어를 외워봅시다! 아무 키나 누르면 시작합니다..." << endl;
    system("pause > nul");
}

void VocaManager::showStatistics() {
    system("cls");
    cout << "==========================================================================" << endl;
    cout << "                             📊 내 학습 통계 리포트" << endl;
    cout << "==========================================================================\n" << endl;

    if (wordList.empty()) {
        cout << "  데이터가 없습니다. 단어를 먼저 추가해주세요!" << endl;
        cout << "\n메인 메뉴로 돌아가려면 아무 키나 누르세요...";
        system("pause > nul");
        return;
    }

    int levelCounts[6] = { 0 };
    for (const auto& w : wordList) {
        if (w.level >= 0 && w.level <= 5) levelCounts[w.level]++;
    }

    cout << " [ 📈 레벨별 단어 분포 ]" << endl;
    for (int i = 5; i >= 0; i--) {
        cout << "  Lv." << i << " | ";
        int barLength = (levelCounts[i] * 40) / wordList.size();
        for (int b = 0; b < barLength; b++) cout << "■";
        for (int b = barLength; b < 40; b++) cout << "□";
        cout << "  " << levelCounts[i] << "개" << endl;
    }

    cout << "\n\n [ ☠️ 마의 오답 TOP 10 ]" << endl;

    vector<Word> sortedWords = wordList;
    sort(sortedWords.begin(), sortedWords.end(), [](const Word& a, const Word& b) {
        return a.failCount > b.failCount;
        });

    cout << "  " << formatLeft("[ 영단어 ]", 20) << " | " << formatLeft("[ 한글 뜻 ]", 35) << " | [오답 횟수]" << endl;
    cout << "--------------------------------------------------------------------------" << endl;

    int topCount = 0;
    for (const auto& w : sortedWords) {
        if (w.failCount == 0) break;
        if (topCount >= 10) break;
        cout << "  " << formatLeft(w.english, 20) << " | " << formatLeft(w.korean, 35) << " | " << w.failCount << "회" << endl;
        topCount++;
    }

    if (topCount == 0) cout << "  🎉 완벽합니다! 한 번도 틀린 단어가 없습니다!" << endl;
    cout << "\n==========================================================================" << endl;
    cout << "메인 메뉴로 돌아가려면 아무 키나 누르세요...";
    system("pause > nul");
}

void VocaManager::showAdminMenu() {
    system("cls");
    cout << "\n=================================" << endl;
    cout << "        🔒 관리자 인증 필요      " << endl;
    cout << "=================================" << endl;

    string adminPassword = "987654321";
    string inputPwd = getSafeString("비밀번호를 입력하세요 (취소: 0): ");

    if (inputPwd == "0") return;

    if (inputPwd != adminPassword) {
        cout << "\n❌ 비밀번호가 틀렸습니다. 접근이 거부되었습니다!" << endl;
        system("pause > nul");
        return;
    }

    while (true) {
        system("cls");
        cout << "=================================" << endl;
        cout << "         🛠️ 시스템 관리자        " << endl;
        cout << "=================================" << endl;
        cout << "1. 🔄 모든 단어 학습 데이터 초기화 (레벨/오답 0으로)" << endl;
        cout << "2. 🗑️ 특정 단어 영구 삭제" << endl;
        cout << "3. 🚫 사용자 계정 영구 삭제" << endl;
        cout << "0. 🔙 메인 메뉴로 돌아가기" << endl;
        cout << "=================================" << endl;

        int choice = getSafeInputInManager("관리자 명령 선택: ");

        if (choice == 0) return;
        else if (choice == 1) {
            cout << "\n⚠️ [초강력 경고] 다음 데이터가 모두 초기화됩니다:" << endl;
            cout << "  - 모든 사용자의 학습 기록 (레벨, 오답 횟수)" << endl;
            cout << "  - 모든 사용자의 누적 점수 및 계정 레벨 (0점, 1레벨로)" << endl;

            string confirm = getSafeString("\n정말 진행하시겠습니까? (y 입력 시 진행): ");

            if (confirm == "y" || confirm == "Y") {
                bool step1 = db.resetAllWordStats();
                bool step2 = db.resetMemberStats();

                if (step1 && step2) {
                    cout << "✅ 성공적으로 모든 데이터와 점수가 초기화되었습니다!" << endl;
                    wordList = db.loadAllWords(currentUser);
                }
                else cout << "❌ 초기화 중 일부 오류가 발생했습니다. DB 상태를 확인하세요." << endl;
            }
            else cout << "초기화를 취소했습니다." << endl;
            system("pause > nul");
        }
        else if (choice == 2) {
            cout << "\n[단어 삭제 모드] (취소하려면 0 입력)" << endl;
            string target = getSafeString("삭제할 영단어를 정확히 입력하세요: ");

            if (target == "0") continue;

            if (db.deleteWord(target)) {
                cout << "✅ [" << target << "] 단어가 DB에서 완전히 삭제되었습니다." << endl;
                wordList = db.loadAllWords(currentUser);
            }
            else cout << "❌ 삭제 실패. 철자가 틀렸거나 DB에 없는 단어입니다." << endl;
            system("pause > nul");
        }
        else if (choice == 3) {
            cout << "\n[사용자 계정 삭제 모드] (취소하려면 0 입력)" << endl;
            string target = getSafeString("삭제할 사용자의 아이디를 정확히 입력하세요: ");

            if (target == "0") continue;

            if (db.deleteUser(target)) cout << "✅ [" << target << "] 사용자의 계정과 모든 학습 기록이 영구 삭제되었습니다." << endl;
            else cout << "❌ 삭제 실패. 존재하지 않는 아이디입니다." << endl;
            system("pause > nul");
        }
        else {
            cout << "❌ 잘못된 관리자 명령입니다." << endl;
            system("pause > nul");
        }
    }
}

bool VocaManager::showAuthMenu() {
    while (true) {
        system("cls");
        cout << "=================================" << endl;
        cout << "        🔐 계정 인증 센터        " << endl;
        cout << "=================================" << endl;
        cout << "1. 🔑 로그인" << endl;
        cout << "2. 📝 회원가입" << endl;
        cout << "0. 🚪 프로그램 종료" << endl;
        cout << "=================================" << endl;

        int choice = getSafeInputInManager("메뉴 선택: ");

        if (choice == 1) {
            string id = getSafeString("▶ 아이디: ");
            string pw = getSafeString("▶ 비밀번호: ");

            if (db.loginUser(id, pw)) {
                currentUser = id;
                wordList = db.loadAllWords(currentUser);

                cout << "\n✅ 로그인 성공! 환영합니다, [" << id << "] 님!" << endl;
                system("pause > nul");
                return true;
            }
            else {
                cout << "\n❌ 로그인 실패! 아이디나 비밀번호를 확인해주세요." << endl;
                system("pause > nul");
            }
        }
        else if (choice == 2) {
            cout << "\n[새 계정 만들기]" << endl;
            string id = getSafeString("▶ 사용할 아이디: ");
            string pw = getSafeString("▶ 사용할 비밀번호: ");

            if (db.registerUser(id, pw)) {
                cout << "\n🎉 회원가입 성공! 이제 가입하신 정보로 로그인해주세요." << endl;
            }
            else {
                cout << "\n⚠️ 회원가입 실패. (이미 누군가 사용 중인 아이디입니다)" << endl;
            }
            system("pause > nul");
        }
        else if (choice == 0) return false;
    }
}

void VocaManager::showRanking() {
    system("cls");
    cout << "==========================================================================" << endl;
    cout << "                             🏆 명예의 전당 (TOP 5)                        " << endl;
    cout << "==========================================================================\n" << endl;

    vector<pair<string, int>> topUsers = db.getTopUsers();

    if (topUsers.empty()) {
        cout << "  아직 등록된 랭커가 없습니다. 첫 번째 랭커가 되어보세요!" << endl;
    }
    else {
        cout << "  " << formatLeft("[ 순위 ]", 10) << " | " << formatLeft("[ 아이디 (칭호) ]", 35) << " | [ 누적 점수 ]" << endl;
        cout << "--------------------------------------------------------------------------" << endl;

        int rank = 1;
        for (const auto& user : topUsers) {
            string medal = " ";
            if (rank == 1) medal = "🥇";
            else if (rank == 2) medal = "🥈";
            else if (rank == 3) medal = "🥉";

            string title;
            int s = user.second;
            if (s >= 1000) title = "[👑 마스터]";
            else if (s >= 500) title = "[💎 다이아]";
            else if (s >= 300) title = "[🥇 골드]";
            else if (s >= 100) title = "[🥈 실버]";
            else title = "[🥉 브론즈]";

            string displayId = title + " " + user.first;

            cout << "  " << formatLeft(to_string(rank) + "등 " + medal, 10) << " | "
                << formatLeft(displayId, 35) << " | " << user.second << " 점" << endl;
            rank++;
        }
    }

    cout << "\n==========================================================================" << endl;
    cout << "메인 메뉴로 돌아가려면 아무 키나 누르세요...";
    system("pause > nul");
}