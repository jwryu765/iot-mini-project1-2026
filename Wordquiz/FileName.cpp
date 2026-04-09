#include <iostream>
#include <mysqlx/xdevapi.h> // MySQL Connector/C++ 9.6 헤더

using namespace std;
using namespace mysqlx;

int main_old() {
    try {
        // 1. 데이터베이스 연결 설정
        // 주소(localhost), 포트(33060 - X Protocol 기본포트), 아이디, 비밀번호
        // 주의: 일반 3306 포트가 아니라 33060 포트를 사용하는 경우가 많습니다.
        Session sess("localhost", 3306, "root", "my123456");

        cout << "데이터베이스 연결에 성공했습니다!" << endl;

        // 2. 현재 서버의 스키마(데이터베이스) 목록 출력 테스트
        cout << "연결된 서버의 스키마 목록:" << endl;
        for (auto schema : sess.getSchemas()) {
            cout << "- " << schema.getName() << endl;
        }

        sess.close(); // 연결 종료

    }
    catch (const mysqlx::Error& err) {
        // 연결 실패 시 에러 메시지 출력
        cerr << "데이터베이스 연결 실패!" << endl;
        cerr << "에러 내용: " << err.what() << endl;
        return 1;
    }

    return 0;
}