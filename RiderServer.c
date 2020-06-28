#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#include "mysql.h"

#define BUFSIZE 1000
#define PORT 8400

//데이터 베이스 접근 정보
const char* server = "localhost";
const char* user = "root";
const char* password = "1234";
const char* database = "netproject";

/* 제너럴 구조체
* 이 구조체의 flag 값은 서버가 어떤 루틴을 처리해야 하는지 알 수 있다.
* flag = 1; 로그인
* flag = 2; 회원 가입
* flag = 3; 주문 정보 요청
* flag = 4; 광고 정보 요청
* flag = 5; 배달 과정 정보 요청
* flag = 6; 해당 주문 승락
* flag = 7; 광고 정보 승락
* flag = 8; 특정 주문 상태 변화
*/
typedef struct flagprotocol {
	int flag;
	char buffer[BUFSIZE];
}FlagProtocol;

//회원 가입 및 로그인 시 필요한 프로토콜
typedef struct adminprotocol {
	int flag;
	char id[40];		//회원 가입 이름
	char name[40];		//아이디.   실패시 원인을 여기다 담음
	char password[40];	//비밀번호
	int result;		// 서버 쪽 응답. (숫자면 ok 0이면 실패)
}AdminProtocol;

//리스트 안에 들어가는 구조체
typedef struct StoreInfo {
	int no;
	char clientaddress[40];
	char storeaddress[40];
	char storename[30];
	char menuname[30];
	char date[20];
}StoreInfo;

//리스트 요청시 필요한 프로토콜
typedef struct ListProtocol {
	int flag;
	int userid;
	int numofstore;
	StoreInfo info[6];
}ListProtocol;

//주문 승락시 필요한 프로토콜
typedef struct AcceptProtocol {
	int flag;
	int itemid;
	int userid;
	int result;
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
			printf("오류다.");
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
		strncpy(response->name, "해당 유저가 없어요", sizeof(char) * 40);
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
			printf("오류다.");
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
		strncpy(registerresult->name, "회원이 존재합니다.", sizeof(char) * 40);
	}
	else {
		char query[200];
		sprintf(query, "insert into rider(id, password, ridername) values (\'%s\', \'%s\', \'%s\');",
			registerbuf->id, registerbuf->password, registerbuf->name);

		mysql_query(conn, "set names euckr");
		if (mysql_query(conn, query) != 0) {
			registerresult->result = 0;
			strncpy(registerresult->name, "생성 실패 다시해주세요", sizeof(char) * 40);
		}
		else {
			registerresult->result = 1;
		}
	}
	mysql_free_result(res);
}

void listprocess(ListProtocol* listbuf ,ListProtocol* listresult, MYSQL* conn) {
	int flag = listbuf->flag;
	char name[50];
	char query[500];
	MYSQL_RES* res = NULL;
	MYSQL_ROW row;

	switch (flag) {
	case 3:
		mysql_query(conn, "set names euckr");
		sprintf(query, "select o._no, o.storename, o.menuname, o.clientaddress, s.address, date_format(time_stamp, \'%%H-%%m\') from ordering as o join store as s on o.storename = s.storename where riderstatus = 0 limit 6");
		mysql_query(conn, query);
		res = mysql_store_result(conn);
		break;
	case 4:
		sprintf(query, "select _no, storename from advertisement where ridername is null limit 6");
		mysql_query(conn, query);
		res = mysql_store_result(conn);
		break;
	case 5:
		mysql_query(conn, "set names euckr");
		sprintf(query, "select o._no, o.storename, o.menuname, o.clientaddress, s.address, date_format(time_stamp, \'%%H-%%m\') from ordering as o join store as s on o.storename = s.storename where riderstatus = 1 and storestatus = 2 and ridername = (select ridername from rider where _no = %d) limit 6", listbuf->userid);
		printf("%s\n", query);
		mysql_query(conn, query);
		res = mysql_store_result(conn);
		break;
	}

	listresult->numofstore = 0;
	while (row = mysql_fetch_row(res)) {
		listresult->info[listresult->numofstore].no = atoi(row[0]);
		strcpy(listresult->info[listresult->numofstore].storename, row[1]);
		if (listresult->flag != 4) {
			strcpy(listresult->info[listresult->numofstore].menuname, row[2]);
			strcpy(listresult->info[listresult->numofstore].clientaddress, row[3]);
			strcpy(listresult->info[listresult->numofstore].storeaddress, row[4]);
			strcpy(listresult->info[listresult->numofstore].date, row[5]);
		}
		listresult->numofstore++;
	}
	mysql_free_result(res);
}

void acceptprocess(AcceptProtocol* acceptbuf, AcceptProtocol* response, MYSQL* conn) {
	int flag = acceptbuf ->flag;
	char query[200];
	MYSQL_RES* res = NULL;
	MYSQL_ROW row;

	sprintf(query, "select ridername from rider where _no = %d", acceptbuf->userid);
	printf("%s", query);
	mysql_query(conn, query);
	res = mysql_store_result(conn);
	row = mysql_fetch_row(res);
	char ridername[40];
	strcpy(ridername, row[0]);
	mysql_free_result(res);

	int num_rows = 0;
	switch (flag) {
	case 6:
		mysql_query(conn, "set names euckr");
		sprintf(query, "update ordering set riderstatus = 1, ridername = \"%s\" where riderstatus = 0 and _no = %d", ridername, acceptbuf->itemid);
		mysql_query(conn, query);
		num_rows = mysql_affected_rows(conn);
		break;
	case 7:
		mysql_query(conn, "set names euckr");
		sprintf(query, "update advertisement set ridername = \"%s\" where ridername is null and _no = %d", ridername, acceptbuf->itemid);
		mysql_query(conn, query);
		num_rows = mysql_affected_rows(conn);
		break;
	case 8:
		sprintf(query, "update ordering set riderstatus = 2 where riderstatus = 1 and storestatus = 2 and _no = %d", acceptbuf->itemid);
		mysql_query(conn, query);
		num_rows = mysql_affected_rows(conn);
		break;
	}
	response->result = num_rows;
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

		FlagProtocol* myflag = (FlagProtocol*)buf;
		char* response = NULL;

		switch (myflag->flag) {
		case 1:
			response = (AdminProtocol*)malloc(sizeof(AdminProtocol));
			loginprocess((AdminProtocol*)buf, (AdminProtocol*)response, conn);
			// 결과를 보낸다.
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
			// 결과를 보낸다.
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
			// 결과를 보낸다.
			retval = sendto(sock, response, sizeof(ListProtocol), 0,
				(SOCKADDR*)&clientaddr, sizeof(clientaddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				continue;
			}
			break;
		case 6: case 7: case 8:
			response = (AcceptProtocol*)malloc(sizeof(AcceptProtocol));
			acceptprocess((AcceptProtocol*)buf, (AcceptProtocol*)response, conn); 
			retval = sendto(sock, response, sizeof(ListProtocol), 0,
				(SOCKADDR*)&clientaddr, sizeof(clientaddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				continue;
			}
			break;
		default: 
			printf("wrong flag error\n");
			break;
		}

		free(response);
	}
	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
