#include <iostream>
#include "VocaManager.h"
using namespace std;

int main() {
    // 🔽 [수정된 부분] 긴 단어 뜻이 밀리지 않도록 콘솔 창 크기를 넓게 고정합니다.
    system("mode con cols=100 lines=30");

    VocaManager manager;
    int choice;

    // [기능 7] 프로그램 시작 시 자동으로 CSV 불러오기
    manager.loadFromCSV("voca.csv");

    while (true) {
        system("cls"); // 화면 지우기 (Windows 기준)
        cout << "=================================" << endl;
        cout << "            Word-Quiz            " << endl;
        cout << "=================================" << endl;
        cout << "1. 📝 일반 퀴즈 시작" << endl;
        cout << "2. 🔥 집중 퀴즈 시작" << endl;
        cout << "3. 📚 전체 단어장 보기" << endl;
        cout << "4. 📖 모르는 단어장 보기" << endl;
        cout << "5. ➕ 단어 수동 추가" << endl;
        cout << "0. 🚪 프로그램 종료 (자동 저장)" << endl;
        cout << "=================================" << endl;
        cout << "메뉴 선택: ";
        cin >> choice;

        if (choice == 1) {
            manager.runQuiz(false);
        }
        else if (choice == 2) {
            manager.runQuiz(true);
        }
        else if (choice == 3) {
            manager.showAllWords(); // 전체 단어장 표로 보기
            system("pause > nul");
        }
        else if (choice == 4) {
            manager.showUnknownWords(); // 모르는 단어장 따로 보기
            system("pause > nul");
        }
        else if (choice == 5) {
            manager.addWord();
            system("pause > nul");
        }
        else if (choice == 0) {
            // [기능 7] 콘솔을 끌 때 자동 저장
            manager.saveToCSV("voca.csv");
            cout << "프로그램을 종료합니다. 단어장이 안전하게 저장되었습니다!" << endl;
            break;
        }
        else {
            cout << "잘못된 입력입니다." << endl;
            system("pause > nul");
        }
    }
    return 0;
}