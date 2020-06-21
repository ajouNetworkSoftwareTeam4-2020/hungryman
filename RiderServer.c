#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#include "mysql.h"

#define BUFSIZE 512
#define PORT 8400

typedef struct adminprotocol {
	char id[40];
	char name[40];		//아이디.   실패시 원인을 여기다 담음
	char password[40];	//비밀번호
	int result;		// 서버 쪽 응답. (숫자면 ok 0이면 실패)
	int howdo;		//로그인 / 회원 가입 구분,
}AdminProtocol;

void err_quit(char* msg)
{
	printf("%s\n", msg);
}

void err_display(char* msg)
{
	printf("%s\n", msg);
}

int main(int argc, char* argv[])
{
	int retval;
	MYSQL* conn; 
	MYSQL_RES* res;   
	MYSQL_ROW row;     

	const char* server = "localhost"; 
	const char* user = "root";   
	const char* password = "1234";  
	const char* database = "netproject"; 
	conn = mysql_init(NULL);

	if (!mysql_real_connect(conn, server,   
		user, password, NULL, 0, NULL, 0)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}
	mysql_query(conn, "use netproject");

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
	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// 데이터 통신에 사용할 변수
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	// 클라이언트와 데이터 통신
	while (1) {
		//로그인 신호를 받는다.
		addrlen = sizeof(clientaddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR*)&clientaddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}

		AdminProtocol* request = (AdminProtocol*)buf;
		AdminProtocol response;

		mysql_query(conn, "select _no from rider where id=\"%s\" and password=\"%s\"", request->id, request->password);
		res = mysql_store_result(conn);
		row = mysql_fetch_row(res);
		int user_no = 0;
		if (row == NULL) {
			user_no = 0;
		}
		else {
			user_no = atoi(row[0]);
		}

		memset(&response, 0x00, sizeof(response));

		if (request->howdo == 1) { //로그인이 된 경우
			response.howdo = 1;
			if (user_no != 0) {
				response.result = user_no;
			}
			else {
				response.result = 0;
				strncpy(response.name, "해당 유저가 없어요", sizeof(40));
			}
		}
		else {
			response.howdo = 2;
			if (user_no != 0) {
				response.result = 0;
				strncpy(response.name, "회원이 존재합니다.", sizeof(40));
			}
			else {
				char query[200];
				sprintf(query, "insert into rider(id, password, ridername) values (\'%s\', \'%s\', \'%s\');", 
					response.id, response.password, response.name);
				if (mysql_query(conn, query) == 0) {
					response.result = 0;
					strncpy(response.name, "생성 실패 다시해주세요", sizeof(40));
				}
				else {
					response.result = 1;
				}
				
			}
		}

		// 결과를 보낸다.
		retval = sendto(sock, (char*)&response, sizeof(AdminProtocol), 0,
			(SOCKADDR*)&clientaddr, sizeof(clientaddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}
	}
	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
