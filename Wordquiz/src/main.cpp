#include <iostream>
#include <string>
#include <sstream> 
#include "VocaManager.h"

using namespace std;

/*
[안전한 정수 입력 처리기]
cin >> inputNum 사용 시 문자가 입력되면 버퍼 스트림이 고장나 무한 루프에 빠지는 치명적 결함이 있습니다.
이를 방어하기 위해 입력을 항상 문자열(getline)로 먼저 받고, stringstream을 통해
정수형으로 안전하게 캐스팅(변환)하여 예외를 완벽히 차단하는 방어적 프로그래밍을 적용했습니다.
*/

// 입력버퍼 오류와 잘못된 차입 입력을 막기 위해, ANSI Escape Code로 이전 줄을 지우고 다시 입력받는 방어적 프로그래밍을 적용했습니다.
int getSafeInput(const string& prompt) {
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
        if (ss >> inputNum && ss.eof()) {
            return inputNum;
        }
        else {
            cout << "⚠️ 잘못된 입력입니다! 숫자를 입력해주세요." << endl;
            system("pause > nul");
            cout << "\x1b[1A\x1b[2K\x1b[1A\x1b[2K";
        }
    }
}

/*
프로그램의 진입점입니다. 
VocaManager 객체를 생성하고, DB 연결을 시도합니다. 연결 실패 시 친절한 메시지를 출력하고 종료합니다. 
연결 성공 시, 인증 메뉴로 넘어가고, 로그인/회원가입이 완료되면 메인 메뉴로 진입합니다. 
메인 메뉴에서는 사용자의 선택에 따라 다양한 기능을 실행할 수 있습니다. 
로그아웃을 선택하면 인증 메뉴로 돌아가고, 프로그램 완전 종료를 선택하면 안전하게 DB에 저장 후 종료됩니다.
*/
int main() {

    VocaManager manager;

    cout << "데이터베이스에 연결하는 중..." << endl;

    if (!manager.init()) {
        cout << "❌ DB 연결에 실패했습니다." << endl;
        system("pause");
        return 1;
    }

    manager.showStartScreen();

    while (true) {
        if (!manager.showAuthMenu()) {
            cout << "프로그램을 종료합니다." << endl;
            return 0;
        }

        while (true) {
            system("cls");
            cout << "=================================" << endl;
            cout << "            Word-Quiz            " << endl;
            cout << "=================================" << endl;
            cout << "1. 📝 일반 퀴즈 시작" << endl;
            cout << "2. 🔥 집중 퀴즈 시작" << endl;
            cout << "3. 📚 전체 단어장 보기" << endl;
            cout << "4. 🎯 오답 단어 몰아보기" << endl;
            cout << "5. 🏆 명예의 전당 (랭킹)" << endl;
            cout << "6. 📊 내 학습 통계 보기" << endl;
            cout << "7. ➕ 단어 수동 추가" << endl;
            cout << "8. 📂 CSV 파일로 단어 추가" << endl;
            cout << "9. 🔓 로그아웃 (계정 전환)" << endl;
            cout << "0. 🚪 프로그램 완전 종료" << endl;
            cout << "=================================" << endl;

            int choice = getSafeInput("메뉴 선택: ");

            if (choice == 1) manager.runQuiz(false);
            else if (choice == 2) manager.runQuiz(true);
            else if (choice == 3) manager.showAllWords();
            else if (choice == 4) manager.showReviewList();
            else if (choice == 5) manager.showRanking();
            else if (choice == 6) manager.showStatistics();
            else if (choice == 7) { manager.addWord(); cout << "\n메인 메뉴로 돌아가려면 아무 키나 누르세요..."; system("pause > nul"); }
            else if (choice == 8) manager.loadFromCSV();
            else if (choice == 99) manager.showAdminMenu();
            else if (choice == 9) {
                cout << "\n✅ 안전하게 로그아웃 되었습니다. 메인 화면으로 돌아갑니다." << endl;
                system("pause > nul");
                break;
            }
            else if (choice == 0) {
                cout << "데이터베이스에 안전하게 저장되었습니다. 프로그램을 종료합니다!" << endl;
                return 0;
            }
            else {
                cout << "❌ 없는 메뉴 번호입니다." << endl;
                system("pause > nul");
            }
        }
    }
    return 0;
}