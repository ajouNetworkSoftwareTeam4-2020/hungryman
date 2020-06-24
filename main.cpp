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

// ���� �Լ� ���� ���
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

    /*�����ͺ��̽� ���� ����*/
    MYSQL* conn; //mysql���� Ŀ�ؼ��� ��µ� ���������� ���Ǵ� ��������.   
    MYSQL_RES* res;  //�������� ���� result���� �޴� ��ġ��������.   
    MYSQL_ROW row;   //�������� ���� ���� �����Ͱ��� ����ִ� ��������.   
    int count = 0;
    char query[255];
    char number[100];
    int check=0; //�ߺ��� id���� üũ
    char category[100]={"\n===ī�װ�===\n1.�ѽ�\n2.���\n3.�߽�\n���� : "};
    char orderinfo[255];

    const char* server = "localhost";  //������ ����ε��� localhost�� �ϸ� �ڱ� ��ǻ�Ͷ� �ǹ̶��ϴ�.   
    const char* user = "root"; //mysql�α��� ���̵��ε���. �⺻������ ���ٸ� ������ ������ root����   
    const char* password = "alswl1997_"; /* set me first */   //password�� �ִºκ��̿���   
    const char* database = "netsof";  //Database �̸��� �־��ִ� �κ��̿���.   

    /*UDP ���� ���� ����*/
    int retval;
    SOCKADDR_IN clientaddr;
    int addrlen;
    char buf[BUFSIZE];
    char buftemp[BUFSIZE]="";
    infotoClient clientinfo;


    /*time�� ���õ� ����*/
    ct = time(NULL);
    tm = *localtime(&ct);
    char ordertime[20];

    // ���� �ʱ�ȭ
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

    conn = mysql_init(NULL); //connection ������ �ʱ�ȭ ���ѿ�.   

    /* Connect to database *///DB���� MYSQL�� ���ӽ�ų������   
    if (!mysql_real_connect(conn, server,    //mysql_real_connect()�Լ��� ������ �����ִ� �Լ�����   
        user, password, NULL, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    mysql_query(conn, "use netsof");

    while (true) {

        /*customer �ߵ����� üũ

        mysql_query(conn, "select * from customer");
        res = mysql_store_result(conn);

        while (row = mysql_fetch_row(res)) {
            for (int i = 0; i < mysql_num_fields(res); i++) {
                //printf("%d��°\n", i);
                printf("%s ", row[i]);
            }
            printf("\n");
        }
        */

        addrlen = sizeof(clientaddr);
        /*ID �Է¹ޱ�*/
        retval = recvfrom(sock, clientinfo.id, sizeof(clientinfo), 0,(SOCKADDR*)&clientaddr, &addrlen);
        if (retval == SOCKET_ERROR) {
            err_display("recv id Error !");
            continue;
        }
        clientinfo.id[retval] = '\0';

        /*Password �Է¹ޱ�*/
        retval = recvfrom(sock, clientinfo.pw, sizeof(clientinfo), 0, (SOCKADDR*)&clientaddr, &addrlen);
        if (retval == SOCKET_ERROR) {
            err_display("recv pw Error !");
            continue;
        }
        clientinfo.pw[retval] = '\0';

        /*���̵�� PW �´��� Ȯ���ϰ�, ������ ���� ���� ��� ������ ���� ����ϰ� ����*/
        mysql_query(conn, "select * from customer");
        res = mysql_store_result(conn);
        while (row = mysql_fetch_row(res)) {
            if (strcmp(row[1], clientinfo.id) == 0) {
                if (strcmp(row[2], clientinfo.pw) == 0) {
                    /*PW�� ID ���� ��쿣 break */
                    break;
                }
                else
                    continue;
            }
            else
                check = 1;
        }

        /*check==1�� ��� ����� �ʿ��� ����. 
        check==0�� ��� ��� �ʿ������ */
        if (check == 1) {
            sprintf(query, "insert into customer (id,password) values (\"%s\",\"%s\");", clientinfo.id, clientinfo.pw);
            mysql_query(conn, query);
            check = 0; //������ְ� check �ٽ� 0����
        }

        /*ī�װ� �����ϰ�, ���� ���� ���� ���*/
        retval = sendto(sock, category, sizeof(category), 0,
            (SOCKADDR*)&clientaddr, sizeof(clientaddr));

        retval = recvfrom(sock, number, sizeof(number), 0, (SOCKADDR*)&clientaddr, &addrlen);
        if (retval == SOCKET_ERROR) {
            err_display("recv id Error !");
            continue;
        }
        number[retval] = '\0';
        clientinfo.selectnum = atoi(number); //���ڿ��� ���� ������ -> ���������� ���� 

  
        /*ī�װ��� ������ store �̸� ���*/

        mysql_query(conn, "select * from store");
        res = mysql_store_result(conn);
        while (row = mysql_fetch_row(res)) {
            /*�ѽĳ�*/
            if (clientinfo.selectnum == 1) {
                if (strcmp(row[5], KOREA_FOOD) == 0) {
                    sprintf(buf, "%d . %s\n", ++count, row[3]);
                    sendto(sock, buftemp, sizeof(buftemp), 0,
                        (SOCKADDR*)&clientaddr, sizeof(clientaddr));
                }
                else
                    continue;
            }
            /*��ĳ�*/
            else if (clientinfo.selectnum == 2) {
                if (strcmp(row[5], WESTERN_FOOD) == 0) {
                    sprintf(buftemp, "%d.%s\n", ++count, row[3]);
                    sendto(sock, buftemp, sizeof(buftemp), 0,
                        (SOCKADDR*)&clientaddr, sizeof(clientaddr));
                }
                else
                    continue;
            }
            /*�߽ĳ�*/
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

        /*���ڷ� �Է� �������� �ߴµ� �ʹ� ��..���ļ�...�� �����̸����� ã�ڽ��ϴ�.
        ���� �̸��� �޴��̸� recv �ޱ�*/
        count = 0;
        //printf("\n�Ĵ��̸� :%s\n", clientinfo.storename);
 
        mysql_query(conn, "select * from store");
        res = mysql_store_result(conn);
        int right = 0;

        while (true) {
            sendto(sock, "�����̸� : ", sizeof("�����̸� : "), 0,
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
                else /*���� �̸��� �ٸ��� �ִ���*/
                    continue;
            }
            if (right == 1)
                break;
            else
                continue;
        }
        sendto(sock, "�޴��̸� : ", sizeof("�޴��̸� : "), 0,
            (SOCKADDR*)&clientaddr, sizeof(clientaddr));

        retval = recvfrom(sock, clientinfo.order, sizeof(clientinfo.order), 0, (SOCKADDR*)&clientaddr, &addrlen);
        clientinfo.order[retval] = '\0';
        sprintf(ordertime, "%d�� %�� %�� %d�� %d��", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,tm.tm_min);


        /*�ֹ� ������Ʈ������Ѵ�.*/
        sprintf(query, "insert into ordering (storename,menuname,ridername,storestatus,riderstatus) values (\"%s\",\"%s\",\"%s\",%d,%d);", clientinfo.storename, clientinfo.order,NULL,NULL,NULL);
        mysql_query(conn, query);

        /*�ֹ� ���� ��½��Ѽ� send �s*/
        sprintf(orderinfo, "\n===%s �ֹ�����===\n�����̸� : %s \n �޴� : %s \n",ordertime, clientinfo.storename, clientinfo.order);
        printf("%s", orderinfo);
     
  

    }
    closesocket(sock);

    // ���� ����
    WSACleanup();
    mysql_close(conn);
}