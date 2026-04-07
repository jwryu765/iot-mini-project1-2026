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

extern vector<Customer> waitingQueue;
extern int nextWaitingNum;

// 시스템 및 UI 헬퍼 함수
void clearScreen();
int getValidIntInput(const string& prompt);
bool isValidPhone(const string& phone);

// 데이터 저장/불러오기
void saveData();
void loadData();

// [제안서 핵심 기능 로직]
int calculateEstimatedTime(int partySize);
void recalculateAllWaitTimes(); // 기능 2: 대기열 전체 시간 실시간 업데이트
void printStatisticsReport();   // 기능 3: 예측 정확도 통계 리포트

// UI 출력 및 메뉴
void printMainMenu();
void printQueueTable();
void registerCustomer();
void viewQueue();
void adminMode();