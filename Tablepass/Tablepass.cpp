#include "TablePass.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <fstream>

// 전역 변수
vector<Customer> waitingQueue;
int nextWaitingNum = 1;
const string ADMIN_PASSWORD = "1234";

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int getValidIntInput(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            return value;
        }
        else {
            cout << "  [오류] 숫자만 입력해주세요!\n";
            cin.clear();
            cin.ignore(1000, '\n');
        }
    }
}

bool isValidPhone(const string& phone) {
    if (phone.length() < 10 || phone.length() > 11) return false;
    for (char c : phone) {
        if (!isdigit(c)) return false;
    }
    return true;
}

void saveData() {
    ofstream fout("waiting_list.txt");
    if (!fout) return;

    fout << nextWaitingNum << "\n";
    for (const auto& c : waitingQueue) {
        fout << c.waitingNum << " "
            << c.phoneNumber << " "
            << c.partySize << " "
            << (int)c.status << " "
            << c.estimatedTime << "\n";
    }
    fout.close();
}

void loadData() {
    ifstream fin("waiting_list.txt");
    if (!fin) return;

    fin >> nextWaitingNum;
    Customer c;
    int statusInt;
    while (fin >> c.waitingNum >> c.phoneNumber >> c.partySize >> statusInt >> c.estimatedTime) {
        c.status = (Status)statusInt;
        waitingQueue.push_back(c);
    }
    fin.close();
}

int calculateEstimatedTime(int partySize) {
    int waitingTeams = 0;
    for (const auto& c : waitingQueue) {
        if (c.status == Status::WAITING) waitingTeams++;
    }
    int baseTimePerTeam = 10;
    int weight = (partySize >= 4) ? 5 : 0;
    return (waitingTeams * baseTimePerTeam) + weight;
}

void printMainMenu() {
    cout << "╔══════════════════════════════════════════╗\n";
    cout << "║        Table-Pass 스마트 웨이팅          ║\n";
    cout << "╠══════════════════════════════════════════╣\n";
    cout << "║                                          ║\n";
    cout << "║   1. 대기 접수 (고객용)                  ║\n";
    cout << "║   2. 대기 현황 (전체보기)                ║\n";
    cout << "║   3. 프로그램 종료                       ║\n";
    cout << "║                                          ║\n";
    cout << "╚══════════════════════════════════════════╝\n";
    cout << "  * 관리자 로그인: 0번 \n\n";
}

// [핵심 변경 사항] 화면 멈춤 없이 표만 출력하는 함수 생성
void printQueueTable() {
    cout << "┌──────┬───────────────┬──────┬────────────┐\n";
    cout << "│ 순번 │   전화번호    │ 인원 │ 예상 시간  │\n";
    cout << "├──────┼───────────────┼──────┼────────────┤\n";

    bool isEmpty = true;
    for (const auto& c : waitingQueue) {
        if (c.status == Status::WAITING) {
            string maskedPhone = c.phoneNumber.substr(0, 3) + "-****-" + c.phoneNumber.substr(c.phoneNumber.length() - 4);

            char buffer[100];
            snprintf(buffer, sizeof(buffer), "│ %2d번 │ %-13s │ %2d명 │ %4d분     │",
                c.waitingNum, maskedPhone.c_str(), c.partySize, c.estimatedTime);

            cout << buffer << "\n";
            isEmpty = false;
        }
    }

    if (isEmpty) {
        cout << "│      현재 대기 중인 팀이 없습니다.       │\n";
    }
    cout << "└──────┴───────────────┴──────┴────────────┘\n";
}

void registerCustomer() {
    clearScreen();
    cout << "┌──────────────────────────────────────────┐\n";
    cout << "│              [ 고 객 접 수 ]             │\n";
    cout << "└──────────────────────────────────────────┘\n";

    string phone;
    while (true) {
        cout << "▶ 전화번호 입력 (기호 없이 숫자만): ";
        cin >> phone;
        if (isValidPhone(phone)) break;
        cout << "  [오류] 10~11자리의 숫자로만 입력해주세요.\n\n";
    }

    int size;
    while (true) {
        size = getValidIntInput("▶ 방문 인원수: ");
        if (size > 0 && size <= 20) break;
        cout << "  [오류] 1명에서 20명 사이로 입력해주세요.\n\n";
    }

    int estimatedTime = calculateEstimatedTime(size);
    waitingQueue.push_back({ nextWaitingNum++, phone, size, Status::WAITING, estimatedTime });
    saveData();

    cout << "\n============================================\n";
    cout << "  [접수 완료] 환영합니다!\n";
    cout << "  대기 번호 : [ " << nextWaitingNum - 1 << " 번 ]\n";
    cout << "  예상 시간 : 약 " << estimatedTime << " 분\n";
    cout << "============================================\n";
    cout << "\n아무 키나 누르면 메인으로 돌아갑니다...\n";
    system("pause > nul");
}

void viewQueue() {
    clearScreen();
    cout << "          [ 현재 매장 대기 현황 ]           \n";
    printQueueTable(); // 단순히 표만 불러옴
    cout << "\n아무 키나 누르면 메인으로 돌아갑니다...\n";
    system("pause > nul");
}

void adminMode() {
    clearScreen();
    cout << "╔══════════════════════════════════════════╗\n";
    cout << "║              [ 관리자 인증 ]             ║\n";
    cout << "╚══════════════════════════════════════════╝\n";

    string inputPw;
    cout << "▶ 비밀번호 입력: ";
    cin >> inputPw;
    if (inputPw != ADMIN_PASSWORD) {
        cout << "\n[경고] 권한이 없습니다.\n";
        system("pause > nul");
        return;
    }

    while (true) {
        clearScreen();
        cout << "╔══════════════════════════════════════════╗\n";
        cout << "║            [ 매장 관리 메뉴 ]            ║\n";
        cout << "╚══════════════════════════════════════════╝\n";
        cout << " 1. 입장 처리    2. 대기 취소(노쇼)    3. 로그아웃\n\n";

        printQueueTable(); // 메뉴 바로 아래에 멈춤 없이 대기열 표시

        int choice = getValidIntInput("\n▶ 메뉴 선택: ");

        if (choice == 3) break;
        if (choice != 1 && choice != 2) continue;

        int targetNum = getValidIntInput("▶ 처리할 대기 번호 입력: ");

        auto it = find_if(waitingQueue.begin(), waitingQueue.end(), [targetNum](const Customer& c) {
            return c.waitingNum == targetNum;
            });

        if (it != waitingQueue.end() && it->status == Status::WAITING) {
            it->status = (choice == 1) ? Status::ENTERED : Status::CANCELED;
            saveData();

            cout << "\n[성공] " << targetNum << "번 고객 "
                << (choice == 1 ? "입장" : "취소") << " 처리 완료!\n";
        }
        else {
            cout << "\n[오류] 대기 중인 번호가 아니거나 존재하지 않습니다.\n";
        }
        system("pause > nul");
    }
}