#include <iostream>
#include <mysql.h>
#include <iomanip>

#pragma comment(lib, "libmysql.lib")

using namespace std;

int main() {
    MYSQL* conn = mysql_init(NULL);
    MYSQL_RES* res;
    MYSQL_ROW row;

    // 접속 정보 확인: DB 이름이 'Wordquiz' (대소문자 주의!)
    if (mysql_real_connect(conn, "127.0.0.1", "root", "my123456", "Wordquiz", 3306, NULL, 0)) {
        cout << "[성공] 데이터베이스에 접속했습니다." << endl;

        // 쿼리문 실행 (마지막에 세미콜론 ; 넣지 마세요!)
        if (mysql_query(conn, "SELECT word, meaning FROM words") == 0) {
            res = mysql_store_result(conn);

            cout << "------------------------------------------" << endl;
            cout << left << setw(15) << "영어 단어" << " | " << "한글 뜻" << endl;
            cout << "------------------------------------------" << endl;

            int count = 0;
            while ((row = mysql_fetch_row(res))) {
                cout << left << setw(15) << row[0] << " | " << row[1] << endl;
                count++;
            }

            if (count == 0) {
                cout << "데이터가 존재하지만 불러오지 못했습니다. 커밋 여부를 확인하세요." << endl;
            }

            mysql_free_result(res);
        }
    }
    else {
        cout << "[연결 실패] 에러: " << mysql_error(conn) << endl;
    }

    mysql_close(conn);
    system("pause");
    return 0;
}