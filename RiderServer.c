#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#include "mysql.h"

#define BUFSIZE 1000
#define PORT 8400

//������ ���̽� ���� ����
const char* server = "localhost";
const char* user = "root";
const char* password = "1234";
const char* database = "netproject";

/* ���ʷ� ����ü
* �� ����ü�� flag ���� ������ � ��ƾ�� ó���ؾ� �ϴ��� �� �� �ִ�.
* flag = 1; �α���
* flag = 2; ȸ�� ����
* flag = 3; �ֹ� ���� ��û
* flag = 4; ���� ���� ��û
* flag = 5; ��� ���� ���� ��û
* flag = 6; �ش� �ֹ� �¶�
* flag = 7; ���� ���� �¶�
* flag = 8; Ư�� �ֹ� ���� ��ȭ
*/
typedef struct flagprotocol {
	int flag;
	char buffer[BUFSIZE];
}FlagProtocol;

//ȸ�� ���� �� �α��� �� �ʿ��� ��������
typedef struct adminprotocol {
	int flag;
	char id[40];		//ȸ�� ���� �̸�
	char name[40];		//���̵�.   ���н� ������ ����� ����
	char password[40];	//��й�ȣ
	int result;		// ���� �� ����. (���ڸ� ok 0�̸� ����)
}AdminProtocol;

//����Ʈ �ȿ� ���� ����ü
typedef struct StoreInfo {
	char storename[45];
	char menuname[50];
}StoreInfo;

//����Ʈ ��û�� �ʿ��� ��������
typedef struct ListProtocol {
	int flag;
	int numofstore;
	StoreInfo info[7];
}ListProtocol;

//�ֹ� �¶��� �ʿ��� ��������
typedef struct AcceptProtocol {
	int flag;
}AcceptProtocol;

void err_quit(char* msg)
{
	printf("%s\n", msg);
}

void err_display(char* msg)
{
	printf("%s\n", msg);
}

void loginprocess(AdminProtocol* loginbuf, AdminProtocol* response, MYSQL* conn) {
	MYSQL_RES* res;
	MYSQL_ROW row;

	char query[200];
	sprintf(query, "select _no from rider where id=\"%s\" and password=\"%s\"", loginbuf->id, loginbuf->password);
	if (mysql_query(conn, query) != 0) {
		{
			printf("������.");
		}
	}
	res = mysql_store_result(conn);
	row = mysql_fetch_row(res);
	int user_no = 0;
	if (row == NULL) {
		user_no = 0;
	}
	else {
		user_no = atoi(row[0]);
	}

	memset(response, 0x00, sizeof(response));
	(response)->flag = 1;
	if (user_no != 0) {
		(response)->result = user_no;
	}
	else {
		(response)->result = 0;
		strncpy(response->name, "�ش� ������ �����", sizeof(char) * 40);
	}
	mysql_free_result(res);
}

void registerprocess(AdminProtocol* registerbuf, AdminProtocol* registerresult, MYSQL* conn) {
	MYSQL_RES* res;
	MYSQL_ROW row;

	char query[200];
	sprintf(query, "select _no from rider where id=\"%s\" and password=\"%s\"", registerbuf->id, registerbuf->password);
	if (mysql_query(conn, query) != 0) {
		{
			printf("������.");
		}
	}

	res = mysql_store_result(conn);
	row = mysql_fetch_row(res);
	int user_no = 0;
	if (row == NULL) {
		user_no = 0;
	}
	else {
		user_no = atoi(row[0]);
	}

	registerresult->flag = 2;
	if (user_no != 0) {
		registerresult->result = 0;
		strncpy(registerresult->name, "ȸ���� �����մϴ�.", sizeof(char) * 40);
	}
	else {
		char query[200];
		sprintf(query, "insert into rider(id, password, ridername) values (\'%s\', \'%s\', \'%s\');",
			registerbuf->id, registerbuf->password, registerbuf->name);

		if (mysql_query(conn, query) != 0) {
			registerresult->result = 0;
			strncpy(registerresult->name, "���� ���� �ٽ����ּ���", sizeof(char) * 40);
		}
		else {
			registerresult->result = 1;
		}
	}
	mysql_free_result(res);
}

void listprocess(ListProtocol* listbuf ,ListProtocol* listresult, MYSQL* conn) {
	int flag = listbuf->flag;
	char query[200];
	MYSQL_RES* res = NULL;
	MYSQL_ROW row;

	switch (flag) {
	case 3:
		sprintf(query, "select storename, menuname from ordering where riderstatus = 0 limit 7");
		mysql_query(conn, query);
		res = mysql_store_result(conn);
		break;
	case 4:
		sprintf(query, "select storename from advertisement where ridername is null limit 7");
		mysql_query(conn, query);
		res = mysql_store_result(conn);
		break;
	case 5:
		sprintf(query, "select storename, menuname from ordering where riderstatus = 1 and storestatus = 2 limit 7");
		mysql_query(conn, query);
		res = mysql_store_result(conn);
		break;
	}

	listresult->numofstore = 0;
	while (row = mysql_fetch_row(res)) {
		strcpy(listresult->info[listresult->numofstore].storename, row[0]);
		if (listresult->flag != 4) {
			strcpy(listresult->info[listresult->numofstore].menuname, row[1]);
		}
		listresult->numofstore++;
	}
	mysql_free_result(res);
}

void acceptprocess(char* buf, char* response, MYSQL* conn) {

}

int main(int argc, char* argv[])
{
	int retval;
	MYSQL* conn;
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

		FlagProtocol* myflag = (FlagProtocol*)buf;
		char* response = NULL;

		switch (myflag->flag) {
		case 1:
			response = (AdminProtocol*)malloc(sizeof(AdminProtocol));
			loginprocess((AdminProtocol*)buf, (AdminProtocol*)response, conn);
			// ����� ������.
			retval = sendto(sock, response, sizeof(AdminProtocol), 0,
				(SOCKADDR*)&clientaddr, sizeof(clientaddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				continue;
			}

			break;
		case 2:
			response = (AdminProtocol*)malloc(sizeof(AdminProtocol));
			registerprocess((AdminProtocol*)buf, (AdminProtocol*)response, conn);
			// ����� ������.
			retval = sendto(sock, response, sizeof(AdminProtocol), 0,
				(SOCKADDR*)&clientaddr, sizeof(clientaddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				continue;
			}
			break;
		case 3: case 4: case 5:
			response = (ListProtocol*)malloc(sizeof(ListProtocol));
			listprocess((ListProtocol*)buf, (ListProtocol*)response, conn);
			// ����� ������.
			retval = sendto(sock, response, sizeof(ListProtocol), 0,
				(SOCKADDR*)&clientaddr, sizeof(clientaddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				continue;
			}
			break;
		case 6: case 7: case 8:
			response = (AcceptProtocol*)malloc(sizeof(AcceptProtocol));
			listprocess((AcceptProtocol*)buf, (AcceptProtocol*)response, conn); break;
		default: break;
		}

		free(response);
	}
	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
