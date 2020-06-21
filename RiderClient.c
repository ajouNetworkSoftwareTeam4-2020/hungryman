// UDPEchoClient.c
// UDP echo client
//
// 과목명: 네트워크소프트웨설계
// 아주대학교 소프트웨어학과
// 이동 멀티미디어 융합 네트워크 연구실 (mmcn.ajou.ac.kr)
//
#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 512
#define PORT 8400

typedef struct adminprotocol {
	char id[40];		//회원 가입 이름
	char name[40];		//아이디.   실패시 원인을 여기다 담음
	char password[40];	//비밀번호
	int result;		// 서버 쪽 응답. (숫자면 ok 0이면 실패)
	int howdo;		//로그인 / 회원 가입 구분, (1이 로그인)
}AdminProtocol;

// 소켓 함수 오류 출력 후 종료
void err_quit(char* msg)
{
	printf("%s\n", msg);
}

// 소켓 함수 오류 출력
void err_display(char* msg)
{
	printf("%s\n", msg);
}

int identity;

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// 소켓 주소 구조체 초기화
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// 데이터 통신에 사용할 변수
	SOCKADDR_IN peeraddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	int len;

	printf("*************************************\n");
	printf("*   Rider Client !!					*\n");
	printf("*************************************\n\n");

	// 서버와 데이터 통신
	while (1) {
		int direction = 0;
		printf("진행 할 작업을 선택하세요 \n");
		printf("1. 로그인\n");
		printf("2. 회원 가입\n");
		scanf("%d", &direction);

		AdminProtocol* request = (AdminProtocol*)malloc(sizeof(AdminProtocol));
		printf("아이디를 입력하세요\n");
		scanf("%s", request->id);
		request->id[strlen(request->id)] = '\0';
		printf("비밀 번호를 입력하세요\n");
		scanf("%s", request->password);
		request->password[strlen(request->password)] = '\0';

		if(direction == 1) {
			request->howdo = 1;
		}
		else {
			request->howdo = 2;
			printf("이름을 입력하세요 \n");
			scanf("%s", request->name);
			request->name[strlen(request->name)] = '\0';
		}

		// 데이터 보내기
		retval = sendto(sock, (char*)request, sizeof(AdminProtocol), 0,
			(SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}
		free(request);

		// 데이터 받기
		AdminProtocol* response;
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR*)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}
		response = (AdminProtocol*)buf;
		
		if (response->result == 0) {
			printf("%s\n", response->name);
			continue;
		}
		else {
			if (response->howdo == 1) {
				printf("로그인 되었습니다.");
				identity = response->result;
				break;
			}
			else {
				printf("회원 가입 되었습니다.");
			}
		}
	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
