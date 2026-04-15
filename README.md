# iot-mini-project1-2026
IoT개발자 과정 미니프로젝트1

## 🎮 Word-Quiz: Multi-User English Voca Manager
> **C++과 MySQL을 활용한 실시간 반응형 영단어 학습 시스템**

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![MySQL](https://img.shields.io/badge/MySQL-4479A1?style=for-the-badge&logo=mysql&logoColor=white)
![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)

본 프로젝트는 단순한 단어 암기 프로그램을 넘어, 데이터베이스 연동을 통한 **사용자별 맞춤 학습 데이터 관리**와 **실시간 비블로킹(Non-blocking) I/O 기술**을 적용한 인터랙티브 퀴즈 시스템 구축을 목표로 합니다.

---

## 1. 프로젝트 개요 (Overview)
학습자가 이미 아는 단어와 미숙달 단어를 데이터로 구분하고, 부족한 단어만 반복 노출하여 학습 효율을 극대화하는 것을 목표로 합니다.

* **기간**: 2026.04.03 ~ 2026.04.16
* **개발 인원**: 1인 (개인 프로젝트)
* **핵심 목표**: 데이터 기반의 효율적인 복습 시스템 및 멀티 유저 환경 구축

## 2. 개발 배경 (Background)
* **현재 문제 상황**
    * **비효율적 복습**: 아는 단어와 모르는 단어가 무작위로 섞여 있어 복습 시 불필요한 시간 낭비 발생.
    * **기억의 휘발성**: 단순히 눈으로 읽는 방식은 출력(Output) 과정이 없어 장기 기억 전환에 한계가 있음.
* **해결 방안**
    * C++의 로직과 MySQL의 영구 저장 기능을 결합하여 사용자의 숙련도를 추적하고 **개인화된 퀴즈 시스템**을 제공합니다.

## 3. 핵심 기능 (Key Features)

### Multi-User System (다중 사용자 지원)
* 회원가입/로그인 기능을 통해 사용자별 학습 진척도(레벨, 누적 오답 횟수)를 독립적으로 관리합니다.

### 에빙하우스 기반 간격 복습 (Spaced Repetition)
* 단어별 숙련도 등급(Lv.0 ~ Lv.5)을 관리하며, 정답 여부에 따라 다음 복습 주기(`nextReview`)를 연장하여 장기 기억 전환을 유도합니다.

### ⏱Real-time Quiz Modes (실시간 퀴즈 모드)
* 주관식, 객관식, 철자 맞추기(Scramble) 모드를 지원합니다.
* **10초 타임어택**: `_kbhit()` 기반의 비블로킹 입력을 활용하여, 입력을 받는 중에도 타이머가 실시간으로 차감되는 긴장감 있는 학습 환경을 제공합니다.

### Data Management & Statistics
* **CSV 연동**: CSV 파일을 통한 대량 단어 추가 및 데이터 정제(Cleaning) 기능을 제공합니다.
* **시각화 통계**: 레벨별 분포도, 오답 TOP 10 리포트, 명예의 전당(랭킹) 시스템을 제공합니다.

## 4. 기술적 도전 및 최적화 (Technical Highlights)

### 1️⃣ Non-blocking I/O Timer (비블로킹 타이머)
* 표준 입력(`cin`)의 블로킹 현상을 해결하기 위해 `_kbhit()` 함수를 도입했습니다. 이를 통해 사용자의 키보드 입력을 대기하는 동안에도 타이머 루프가 중단 없이 실시간으로 작동합니다.

### 2️⃣ Rendering Optimization (화면 갱신 최적화)
* 전체 화면을 지우는 `system("cls")` 대신 **ANSI Escape Code**(`\33[2K`, `\r`)를 활용했습니다. 필요한 줄만 실시간으로 갱신하여 콘솔 특유의 깜빡임(Flickering) 현상을 95% 이상 제거했습니다.

### 3️⃣ Database Normalization & Join (DB 최적화, 위 아키텍처의 DBLayer 참조)
* 단어 사전(`words`)과 사용자 기록(`user_words`) 테이블을 분리 설계했습니다. `LEFT JOIN`과 `IFNULL`을 조합하여 학습 기록이 없는 데이터도 안전하게 병합하는 효율적인 쿼리를 작성했습니다.

### 4️⃣ Memory Safety & Security (안전한 설계)
* **SQL Injection 방어**: `mysql_real_escape_string`을 적용하여 악의적인 쿼리 조작을 차단했습니다.
* **RAII 패턴**: 이스케이프 처리 시 `std::vector`를 사용하여 예외 발생 시에도 메모리 누수(Memory Leak)가 발생하지 않도록 설계했습니다.


## 5. 기술 스택 (Tech Stack)
- **Frontend**: Windows Console (ANSI Escape Code 활용)
- **Backend**: C++ (STL, Chrono, Conio 활용)
- **Database**: MySQL 8.0 (C API Connector)
- **Tool**: Visual Studio 2026, DBeaver

## 6. 기대 효과 (Expected Effects)
- 사용자 측면
    - 학습 시간 단축: 아는 단어는 과감히 건너뛰고 취약한 부분만 반복 학습하여 장기 기억 형성 유도.
    - 학습 동기 부여: 수치화된 등급을 통해 본인의 성장 과정을 시각적으로 확인 가능.
- 기술적 성과
    - 데이터 설계: 관계형 데이터베이스(RDBMS) 설계 및 LEFT JOIN 등 복합 쿼리 활용 능력 향상.
    - 시스템 프로그래밍: 입출력 버퍼를 직접 제어하는 비블로킹 I/O 구현 능력 확보.

## 7. 개발 일정 (Timeline)
- 1주차: 기획, DB 스키마 설계, C++ 로직 개발 및 DB 연동
- 2주차: 타임어택 기능 테스트, 최종 보완 및 발표 준비

## 8. 리스크 및 대응 (Risk Management)
- 리스크 1: 표준 입력(cin)의 블로킹으로 인한 실시간성 저해
    - 문제점: cin을 사용할 경우 사용자가 입력을 완료(Enter)할 때까지 프로그램이 멈춰, 퀴즈의 핵심인 실시간 타이머(10초)가 작동하지 않는 문제 발생.

    - 대응: conio.h의 _kbhit() 함수를 도입하여 비블로킹(Non-blocking) I/O를 구현함. 입력을 대기하는 중에도 루프가 멈추지 않고 계속해서 시간을 계산하도록 설계하여 실시간 타임어택 기능을 완성함.

- 리스크 2: 화면 전체 갱신(system("cls"))으로 인한 깜빡임(Flickering)
    - 문제점: 매 초 타이머를 갱신할 때마다 화면 전체를 지우고 새로 그려, 눈이 아플 정도의 심한 화면 떨림 현상이 발생하여 사용자 경험(UX) 저하.

    - 대응: system("cls") 대신 ANSI 이스케이프 시퀀스(\33[2K, \r)를 활용한 부분 렌더링 기법을 적용. 현재 줄만 지우고 커서를 앞으로 옮기는 최적화를 통해 깜빡임 없는 안정적인 UI를 구축함.

- 리스크 3: 사용자 입력값에 의한 SQL Injection 보안 취약점
    - 문제점: 단어 추가나 회원가입 시 사용자가 작은따옴표(') 등 특수문자를 입력하면 SQL 쿼리가 조작되거나 DB 에러가 발생할 위험 존재.

    - 대응: mysql_real_escape_string 함수를 사용하여 모든 사용자 입력값에 이스케이프(Escape) 처리를 수행함. 이를 통해 보안성을 강화하고 데이터베이스의 안정적인 트랜잭션을 확보함.

## 9. 파일 구성 (File Tree)
```text
WordQuiz
├── src                         # 핵심 소스 코드 (.cpp, .h)
│   ├── main.cpp               # 진입점 및 메인 루프
│   ├── VocaManager.cpp/.h     # 비즈니스 로직 (퀴즈, 통계)
│   ├── DBManager.cpp/.h       # DB 연동 및 보안 처리
│   └── Word.h                 # 데이터 모델 클래스
├── data                        # 데이터베이스 관련
│   ├── WordQuiz_dump.sql      # DB 스키마 백업
│   └── voca_list.csv          # 샘플 단어 데이터
└── README.md                   # 프로젝트 문서
```
