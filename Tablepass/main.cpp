#include "TablePass.h"
#include <iostream>

int main() {
    // Windows 콘솔창 UTF-8 인코딩 강제 설정
    system("chcp 65001 > nul");

    // 프로그램 시작 시 기존 데이터 불러오기
    loadData();

    while (true) {
        clearScreen();
        printMainMenu();

        int menu = getValidIntInput("▶ 원하시는 메뉴를 선택하세요: ");

        if (menu == 1) registerCustomer();
        else if (menu == 2) viewQueue();
        else if (menu == 3) break;
        else if (menu == 0) adminMode();
    }
    return 0;
}