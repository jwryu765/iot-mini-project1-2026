#include <iostream>
#include "VocaManager.h"

using namespace std;

int main() {
    // 1. 윈도우 콘솔의 인코딩을 강제로 UTF-8(65001)로 변경하여 한글 깨짐 완벽 방지
    system("chcp 65001 > nul");

    VocaManager manager;
    int choice;

    cout << "데이터베이스에 연결하는 중..." << endl;

    // DB 연결 및 단어 데이터 불러오기
    if (!manager.init()) {
        cout << "❌ DB 연결에 실패했습니다. 프로그램을 종료합니다." << endl;
        system("pause");
        return 1;
    }

    while (true) {
        system("cls"); // 화면 지우기
        cout << "=================================" << endl;
        cout << "            Word-Quiz            " << endl;
        cout << "=================================" << endl;
        cout << "1. 📝 일반 퀴즈 시작" << endl;
        cout << "2. 🔥 집중 퀴즈 시작" << endl;
        cout << "3. 📚 전체 단어장 보기" << endl;
        cout << "4. 📖 모르는 단어장 보기" << endl;
        cout << "5. ➕ 단어 수동 추가" << endl;
        cout << "0. 🚪 프로그램 종료 (DB 실시간 저장)" << endl;
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
            manager.showAllWords();
            system("pause > nul");
        }
        else if (choice == 4) {
            manager.showUnknownWords();
            system("pause > nul");
        }
        else if (choice == 5) {
            manager.addWord();
            system("pause > nul");
        }
        else if (choice == 0) {
            cout << "프로그램을 종료합니다. 모든 데이터는 안전하게 DB에 저장되어 있습니다!" << endl;
            break;
        }
        else {
            cout << "잘못된 입력입니다." << endl;
            system("pause > nul");
        }
    }
    return 0;
}