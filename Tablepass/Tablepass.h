#pragma once
#include <vector>
#include <string>

using namespace std;

// 데이터 타입 정의
enum class Status { WAITING, ENTERED, CANCELED };

struct Customer {
    int waitingNum;
    string phoneNumber;
    int partySize;
    Status status;
    int estimatedTime;
};

// 전역 변수 공유 선언
extern vector<Customer> waitingQueue;
extern int nextWaitingNum;

// 시스템 및 UI 헬퍼 함수
void clearScreen();
int getValidIntInput(const string& prompt);
bool isValidPhone(const string& phone);

// 데이터 저장/불러오기
void saveData();
void loadData();

// 핵심 비즈니스 로직 및 UI 출력
int calculateEstimatedTime(int partySize);
void printMainMenu();
void printQueueTable(); // [추가] 표 그리기 전용 함수
void registerCustomer();
void viewQueue();
void adminMode();
