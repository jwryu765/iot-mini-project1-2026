# iot-mini-project1-2026
IoT개발자 과정 미니프로젝트1

## 🎮 Word-Quiz: Multi-User English Voca Manager
> **C++과 MySQL을 활용한 실시간 반응형 영단어 학습 시스템**

본 프로젝트는 단순한 단어 암기 프로그램을 넘어, 데이터베이스 연동을 통한 사용자별 맞춤 학습 데이터 관리와 실시간 Non-blocking I/O 기술을 적용한 인터랙티브 퀴즈 시스템 구축을 목표로 합니다.

### 📝 프로젝트 개요 (Overview)
- 학습자가 이미 아는 단어와 미숙달 단어를 데이터로 구분하고, 부족한 단어만 반복 노출하여 학습 효율을 극대화하는 것을 목표로 합니다.
    - 기간: 2026.03 ~ 2026.04.16 (발표 예정)
    - 개발 인원: 1인 (개인 프로젝트)
    - 핵심 목표: 데이터 기반의 효율적인 복습 시스템 구축

### 💡 개발 배경 (Background)
- 현재 문제 상황
    - 비효율적 복습: 아는 단어와 모르는 단어가 무작위로 섞여 있어 복습 시 불필요한 시간 낭비 발생.
    - 기억의 휘발성: 단순히 눈으로 읽는 방식은 출력(Output) 과정이 없어 장기 기억 전환에 한계가 있음.
- 해결 방안
    - C++의 로직과 MySQL의 영구 저장 기능을 결합하여, 사용자의 숙련도를 추적하고 개인화된 퀴즈 시스템을 제공합니다.

### 🚀 Key Features
- **Multi-User System**: 회원가입/로그인 기능을 통해 사용자별 학습 진척도(레벨, 오답 횟수) 독립 관리
- **Real-time Quiz Modes**: 
  - 주관식, 객관식, 철자 맞추기(Scramble) 모드
  - **10초 타임어택**: `_kbhit()` 기반의 비블로킹 입력을 활용한 실시간 타이머 구현
- **Database Integration**: MySQL 연동을 통한 영구적 데이터 보존 및 `LEFT JOIN`을 활용한 효율적인 데이터 병합
- **Data Management**: CSV 파일을 통한 대량 단어 추가 및 데이터 정제(Cleaning) 기능
- **Visual Statistics**: 사용자 레벨 분포도 및 오답 TOP 10 통계 리포트 제공

### 🛠 Tech Stack
- **Frontend**: Windows Console (ANSI Escape Code 활용)
- **Backend**: C++ (STL, Chrono, Conio 활용)
- **Database**: MySQL 8.0 (C API Connector)
- **Toolt**: Visual Studio 2026, DBeaver

### 📊 기대 효과 (Expected Effects)
- 사용자 측면
    - 학습 시간 단축: 아는 단어는 과감히 건너뛰고 취약한 부분만 반복 학습하여 장기 기억 형성 유도.
    - 학습 동기 부여: 수치화된 등급을 통해 본인의 성장 과정을 시각적으로 확인 가능.
- 기술적 성과
    - 데이터 설계: 관계형 데이터베이스(RDBMS) 설계 및 LEFT JOIN 등 복합 쿼리 활용 능력 향상.
    - 시스템 프로그래밍: 입출력 버퍼를 직접 제어하는 비블로킹 I/O 구현 능력 확보.

### 📅 개발 일정 (Timeline)
- 1주차: 기획, DB 스키마 설계, C++ 로직 개발 및 DB 연동
- 2주차: UI 최적화 및 타임어택 기능 테스트, 최종 보완 및 발표 준비

### 🚨 리스크 및 대응 (Risk Management)
- 문제: 복잡한 숙련도 업데이트 로직 구현 시 발생할 수 있는 프로그램 꼬임 현상.
- 대응: 초기 단계에서는 bool 타입의 단순 이분법적 구분을 우선 구현하고, 안정성 검증 후 단계적 등급(Level 0~5) 시스템으로 고도화함.

### 💡 Technical Highlights (발표 핵심 내용)

- Non-blocking I/O Timer
    - 표준 입력(`cin`)의 블로킹 현상을 해결하기 위해 `_kbhit()` 함수를 도입하였습니다. 이를 통해 사용자의 입력을 기다리는 동안에도 10초 타이머가 실시간으로 흐르는 반응형 시스템을 구축했습니다.

- Rendering Optimization
    - 콘솔의 전체 화면을 지우는 `system("cls")` 대신 ANSI Escape Code(`\33[2K`, `\r`)를 활용했습니다. 필요한 입력 줄만 실시간으로 갱신하여 화면 깜빡임(Flickering) 현상을 95% 이상 제거했습니다.

- Database Normalization & Join
    - 단어 사전(`words`)과 사용자 기록(`user_words`) 테이블을 분리 설계했습니다. `LEFT JOIN`과 `IFNULL` 함수를 조합하여 학습 기록이 없는 데이터도 안전하게 병합하는 효율적인 쿼리를 작성했습니다.