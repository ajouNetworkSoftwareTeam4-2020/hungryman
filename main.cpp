#include <stdio.h>
#include <winsock.h>
#include <string.h>
#include <time.h>
#include "mysql.h"

#define KOREA_FOOD "koreanfood"
#define CHINA_FOOD "chinafood"
#define WESTERN_FOOD "westernfood"

#define BUFSIZE 512

time_t ct;
struct tm tm;

void err_quit(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(-1);
}

// 소켓 함수 오류 출력
void err_display(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
    LocalFree(lpMsgBuf);
}
struct infotoClient {
    char id[20];
    char pw[20];
    char storename[20];
    char order[20];
    int selectnum;
};


int main(void) {

    /*데이터베이스 관련 선언*/
    MYSQL* conn; //mysql과의 커넥션을 잡는데 지속적으로 사용되는 변수에요.   
    MYSQL_RES* res;  //쿼리문에 대한 result값을 받는 위치변수에요.   
    MYSQL_ROW row;   //쿼리문에 대한 실제 데이터값이 들어있는 변수에요.   
    int count = 0;
    char query[255];
    char number[100];
    int check=0; //중복된 id인지 체크
    char category[100]={"\n===카테고리===\n1.한식\n2.양식\n3.중식\n선택 : "};
    char orderinfo[255];

    const char* server = "localhost";  //서버의 경로인데요 localhost로 하면 자기 컴퓨터란 의미랍니다.   
    const char* user = "root"; //mysql로그인 아이디인데요. 기본적으로 별다른 설정이 없으면 root에요   
    const char* password = "alswl1997_"; /* set me first */   //password를 넣는부분이에요   
    const char* database = "netsof";  //Database 이름을 넣어주는 부분이에요.   

    /*UDP 관련 변수 선언*/
    int retval;
    SOCKADDR_IN clientaddr;
    int addrlen;
    char buf[BUFSIZE];
    char buftemp[BUFSIZE]="";
    infotoClient clientinfo;


    /*time에 관련된 변수*/
    ct = time(NULL);
    tm = *localtime(&ct);
    char ordertime[20];

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return -1;

    // socket()
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8000);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    retval = bind(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    conn = mysql_init(NULL); //connection 변수를 초기화 시켜요.   

    /* Connect to database *///DB없이 MYSQL만 접속시킬꺼에요   
    if (!mysql_real_connect(conn, server,    //mysql_real_connect()함수가 연결을 시켜주는 함수에요   
        user, password, NULL, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    mysql_query(conn, "use netsof");

    while (true) {

        /*customer 잘들어갔는지 체크

        mysql_query(conn, "select * from customer");
        res = mysql_store_result(conn);

        while (row = mysql_fetch_row(res)) {
            for (int i = 0; i < mysql_num_fields(res); i++) {
                //printf("%d번째\n", i);
                printf("%s ", row[i]);
            }
            printf("\n");
        }
        */

        addrlen = sizeof(clientaddr);
        /*ID 입력받기*/
        retval = recvfrom(sock, clientinfo.id, sizeof(clientinfo), 0,(SOCKADDR*)&clientaddr, &addrlen);
        if (retval == SOCKET_ERROR) {
            err_display("recv id Error !");
            continue;
        }
        clientinfo.id[retval] = '\0';

        /*Password 입력받기*/
        retval = recvfrom(sock, clientinfo.pw, sizeof(clientinfo), 0, (SOCKADDR*)&clientaddr, &addrlen);
        if (retval == SOCKET_ERROR) {
            err_display("recv pw Error !");
            continue;
        }
        clientinfo.pw[retval] = '\0';

        /*아이디랑 PW 맞는지 확인하고, 있으면 상점 전송 고고리 없으면 새로 등록하고 전송*/
        mysql_query(conn, "select * from customer");
        res = mysql_store_result(conn);
        while (row = mysql_fetch_row(res)) {
            if (strcmp(row[1], clientinfo.id) == 0) {
                if (strcmp(row[2], clientinfo.pw) == 0) {
                    /*PW랑 ID 같은 경우엔 break */
                    break;
                }
                else
                    continue;
            }
            else
                check = 1;
        }

        /*check==1일 경우 등록이 필요한 경우다. 
        check==0인 경우 등록 필요없슴둥 */
        if (check == 1) {
            sprintf(query, "insert into customer (id,password) values (\"%s\",\"%s\");", clientinfo.id, clientinfo.pw);
            mysql_query(conn, query);
            check = 0; //등록해주고 check 다시 0으로
        }

        /*카테고리 전달하고, 전달 받은 숫자 출력*/
        retval = sendto(sock, category, sizeof(category), 0,
            (SOCKADDR*)&clientaddr, sizeof(clientaddr));

        retval = recvfrom(sock, number, sizeof(number), 0, (SOCKADDR*)&clientaddr, &addrlen);
        if (retval == SOCKET_ERROR) {
            err_display("recv id Error !");
            continue;
        }
        number[retval] = '\0';
        clientinfo.selectnum = atoi(number); //문자열로 받은 선택지 -> 정수형으로 변경 

  
        /*카테고리랑 맞으면 store 이름 출력*/

        mysql_query(conn, "select * from store");
        res = mysql_store_result(conn);
        while (row = mysql_fetch_row(res)) {
            /*한식냠*/
            if (clientinfo.selectnum == 1) {
                if (strcmp(row[5], KOREA_FOOD) == 0) {
                    sprintf(buf, "%d . %s\n", ++count, row[3]);
                    sendto(sock, buftemp, sizeof(buftemp), 0,
                        (SOCKADDR*)&clientaddr, sizeof(clientaddr));
                }
                else
                    continue;
            }
            /*양식냠*/
            else if (clientinfo.selectnum == 2) {
                if (strcmp(row[5], WESTERN_FOOD) == 0) {
                    sprintf(buftemp, "%d.%s\n", ++count, row[3]);
                    sendto(sock, buftemp, sizeof(buftemp), 0,
                        (SOCKADDR*)&clientaddr, sizeof(clientaddr));
                }
                else
                    continue;
            }
            /*중식냠*/
            else if (clientinfo.selectnum == 3) {
                if (strcmp(row[5], CHINA_FOOD) == 0) {
                    sprintf(buftemp, "%d . %s\n", ++count, row[3]);
                    sendto(sock, buftemp, sizeof(buftemp), 0,
                        (SOCKADDR*)&clientaddr, sizeof(clientaddr));
                }
                else
                    continue;
            }
        }

        /*숫자로 입력 받을려고 했는데 너무 빣..ㅅㅔ서...걍 상점이름으로 찾겠습니다.
        상점 이름과 메뉴이름 recv 받기*/
        count = 0;
        //printf("\n식당이름 :%s\n", clientinfo.storename);
 
        mysql_query(conn, "select * from store");
        res = mysql_store_result(conn);
        int right = 0;

        while (true) {
            sendto(sock, "상점이름 : ", sizeof("상점이름 : "), 0,
                (SOCKADDR*)&clientaddr, sizeof(clientaddr));
            retval = recvfrom(sock, clientinfo.storename, sizeof(clientinfo.storename), 0, (SOCKADDR*)&clientaddr, &addrlen);
            clientinfo.storename[retval] = '\0';
            while (row = mysql_fetch_row(res)) {
                if (strcmp(clientinfo.storename, row[3]) == 0) {
                    sendto(sock, "quit ", sizeof("quit"), 0,
                        (SOCKADDR*)&clientaddr, sizeof(clientaddr));
                    right = 1;
                    break;
                }
                else /*상점 이름이 다른게 있는지*/
                    continue;
            }
            if (right == 1)
                break;
            else
                continue;
        }
        sendto(sock, "메뉴이름 : ", sizeof("메뉴이름 : "), 0,
            (SOCKADDR*)&clientaddr, sizeof(clientaddr));

        retval = recvfrom(sock, clientinfo.order, sizeof(clientinfo.order), 0, (SOCKADDR*)&clientaddr, &addrlen);
        clientinfo.order[retval] = '\0';
        sprintf(ordertime, "%d년 %월 %일 %d시 %d분", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,tm.tm_min);


        /*주문 업데이트해줘야한다.*/
        sprintf(query, "insert into ordering (storename,menuname,ridername,storestatus,riderstatus) values (\"%s\",\"%s\",\"%s\",%d,%d);", clientinfo.storename, clientinfo.order,NULL,NULL,NULL);
        mysql_query(conn, query);

        /*주문 내역 출력시켜서 send 뾱*/
        sprintf(orderinfo, "\n===%s 주문정보===\n상점이름 : %s \n 메뉴 : %s \n",ordertime, clientinfo.storename, clientinfo.order);
        printf("%s", orderinfo);
     
  

    }
    closesocket(sock);

    // 윈속 종료
    WSACleanup();
    mysql_close(conn);
}