#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#include "mysql.h"

#define BUFSIZE 512
#define PORT 8400

typedef struct adminprotocol {
	char id[40];
	char name[40];		//���̵�.   ���н� ������ ����� ����
	char password[40];	//��й�ȣ
	int result;		// ���� �� ����. (���ڸ� ok 0�̸� ����)
	int howdo;		//�α��� / ȸ�� ���� ����,
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
	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// ������ ��ſ� ����� ����
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	// Ŭ���̾�Ʈ�� ������ ���
	while (1) {
		//�α��� ��ȣ�� �޴´�.
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

		if (request->howdo == 1) { //�α����� �� ���
			response.howdo = 1;
			if (user_no != 0) {
				response.result = user_no;
			}
			else {
				response.result = 0;
				strncpy(response.name, "�ش� ������ �����", sizeof(40));
			}
		}
		else {
			response.howdo = 2;
			if (user_no != 0) {
				response.result = 0;
				strncpy(response.name, "ȸ���� �����մϴ�.", sizeof(40));
			}
			else {
				char query[200];
				sprintf(query, "insert into rider(id, password, ridername) values (\'%s\', \'%s\', \'%s\');", 
					response.id, response.password, response.name);
				if (mysql_query(conn, query) == 0) {
					response.result = 0;
					strncpy(response.name, "���� ���� �ٽ����ּ���", sizeof(40));
				}
				else {
					response.result = 1;
				}
				
			}
		}

		// ����� ������.
		retval = sendto(sock, (char*)&response, sizeof(AdminProtocol), 0,
			(SOCKADDR*)&clientaddr, sizeof(clientaddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}
	}
	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
