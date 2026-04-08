#include <iostream>
#include "VocaManager.h"

using namespace std;

int main() {
    VocaManager manager;
    int choice;

    while (true) {
        system("cls"); // 화면 지우기
        cout << "=================================" << endl;
        cout << "            Word-Quiz            " << endl;
        cout << "=================================" << endl;
        cout << "1. 📝 일반 퀴즈 시작" << endl;
        cout << "2. 🔥 집중 퀴즈 시작" << endl;
        cout << "3. 📖 모르는 단어장 보기" << endl;
        cout << "4. 📂 단어장 불러오기(CSV)" << endl;
        cout << "5. ➕ 단어 수동 추가" << endl;
        cout << "0. 🚪 프로그램 종료" << endl;
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
            manager.showUnknownWords();
            system("pause > nul");
        }
        else if (choice == 4) {
            manager.loadFromCSV("voca.csv");
            system("pause > nul");
        }
        else if (choice == 5) {
            manager.addWord();
            system("pause > nul");
        }
        else if (choice == 0) {
            cout << "프로그램을 종료합니다. 수고하셨습니다!" << endl;
            break;
        }
        else {
            cout << "잘못된 입력입니다." << endl;
            system("pause > nul");
        }
    }
    return 0;
}