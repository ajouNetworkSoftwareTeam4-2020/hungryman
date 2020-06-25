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

//안 넘칠 것 같지만 오류 뜨면 넘친거다.
#define BUFSIZE 1000
#define PORT 8400

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
	char storename[45];
	char menuname[50];
}StoreInfo;

//리스트 요청시 필요한 프로토콜
typedef struct ListProtocol {
	int flag;
	int numofstore;
	StoreInfo info[7];
}ListProtocol;

//주문 승락시 필요한 프로토콜
typedef struct AcceptProtocol {
	int flag;
}AcceptProtocol;

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

	printf("	*************************************\n");
	printf("	*   Rider Client !!		    *\n");
	printf("	*************************************\n\n");

	// 서버와 데이터 통신
	while (1) {
		int direction = 0;
		printf("  ********************************\n");
		printf("  *   진행 할 작업을 선택하세요  *\n");
		printf("  *   1. 로그인                  *\n");
		printf("  *   2. 회원 가입               *\n");
		printf("  ********************************\n");
		printf("원하시는 작업 번호를 입력하세요 => ");
		scanf("%d", &direction);
		printf("\n");

		printf("\n---------------------------------\n");
		AdminProtocol* request = (AdminProtocol*)malloc(sizeof(AdminProtocol));
		printf("  아이디를 입력하세요 => ");
		scanf("%s", request->id);
		request->id[strlen(request->id)] = '\0';
		printf("  비밀 번호를 입력하세요 =>");
		scanf("%s", request->password);
		request->password[strlen(request->password)] = '\0';

		if(direction == 1) {
			request->flag = 1;
		}
		else {
			request->flag = 2;
			printf("  이름을 입력하세요  => ");
			scanf("%s", request->name);
			request->name[strlen(request->name)] = '\0';
		}
		printf("---------------------------------\n");

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
			if (response->flag == 1) {
				printf("로그인 되었습니다. 다음 기능으로 넘어갑니다.\n");
				identity = response->result;
				break;
			}
			else {
				printf("회원 가입 되었습니다.\n");
			}
		}
		printf("---------------------------------\n\n");
	}

	while (1) {
		printf("\n");
		int whatdo;
		printf("  **********************************\n");
		printf("  *   할 일을 선택하세요~!         *\n");
		printf("  * 1. 주문 접수하기               *\n");
		printf("  * 2. 광고 등록하기               *\n");
		printf("  * 3. 배달 완료하기               *\n");
		printf("  **********************************\n");
		printf("원하시는 작업 번호를 입력하세요 => ");
		scanf("%d", &whatdo);

		ListProtocol* request = (ListProtocol*)malloc(sizeof(ListProtocol));
		switch (whatdo) {
		case 1:
			request->flag = 3;
			retval = sendto(sock, (char*)request, sizeof(ListProtocol), 0,
				(SOCKADDR*)&serveraddr, sizeof(serveraddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				continue;
			}
			free(request);
			break;
		case 2:
			request->flag = 4;
			retval = sendto(sock, (char*)request, sizeof(ListProtocol), 0,
				(SOCKADDR*)&serveraddr, sizeof(serveraddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				continue;
			}
			free(request);
			break;
		case 3:
			request->flag = 5;
			retval = sendto(sock, (char*)request, sizeof(ListProtocol), 0,
				(SOCKADDR*)&serveraddr, sizeof(serveraddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				continue;
			}
			free(request);
			break;
		default:
			printf("정의하지 않은 활동입니다. 다시 선택하세요\n\n");
			continue;
		}

		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR*)&peeraddr, &addrlen);

		ListProtocol* returnlist = buf;

		printf("\n------- 선택 리스트 ---------------------------------\n");
		for (int i = 0; i < returnlist->numofstore; i++) {
			if (whatdo != 2) {
				printf("  %d =>  가게 이름 : %s , 주문 이름 : %s \n", 
					i+1 , returnlist->info[i].storename, returnlist->info[i].menuname);
			}
			else {
				printf("  %d => 가게 이름 : %s \n",
					i+1, returnlist->info[i].storename);
			}
		}
		printf("------------------------------------------------------\n");

		printf("\n ----- 원하는 리스트 번호를 선택하세요 -----\n");
	}
	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
