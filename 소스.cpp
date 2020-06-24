#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 512

struct infotoClient {
	char id[20];
	char pw[20];
	char storename[20];
	char order[20];
	char selectnum[100];
};
// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	//MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
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

int main(int argc, char* argv[])
{
	int retval;
	infotoClient clientinfo;
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
	serveraddr.sin_port = htons(8000);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// 데이터 통신에 사용할 변수
	SOCKADDR_IN peeraddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	int len;

	// 서버와 데이터 통신
	while (1) {
		// 데이터 입력


		/*ID 입력*/
		printf("ID 입력 : ");
		scanf("%s", clientinfo.id);
		len = strlen(clientinfo.id);
		clientinfo.id[len] = '\0';

		retval = sendto(sock, clientinfo.id, strlen(clientinfo.id), 0,
			(SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}

		/*PW 입력*/
		printf("PW 입력 : ");
		scanf("%s", clientinfo.pw);
		len = strlen(clientinfo.pw);
		clientinfo.pw[len] = '\0';
		retval = sendto(sock, clientinfo.pw, strlen(clientinfo.pw), 0,
			(SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}


		/*카테고리 정보 받기*/
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR*)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}
		buf[retval] = '\0';
		printf("%s ", buf);

		/*카테고리 선택해서 send보내기 */
		scanf("%s", clientinfo.selectnum);
		len = strlen(clientinfo.selectnum);
		clientinfo.selectnum[len] = '\0';
		retval = sendto(sock, clientinfo.selectnum, len, 0,
			(SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}

		printf("\n");
		/*상점 정보 받고 출력하기*/
		//while문을 send 없을 때 동안 돌아야 한다. -> 선택 : 멘트 들어올 때 중단 할 수 있도록 했다.
		while (1) {
			retval = recvfrom(sock, buf, BUFSIZE, 0,
				(SOCKADDR*)&peeraddr, &addrlen);
			if (retval == SOCKET_ERROR) {
				err_display("recvfrom()");
				continue;
			}
			buf[retval] = '\0';
			printf("%s", buf);
			if (strcmp(buf, "상점이름 : ") == 0) {
				scanf("%s", clientinfo.storename);
				len = strlen(clientinfo.storename);
				clientinfo.storename[len] = '\0';
				retval = sendto(sock, clientinfo.storename, strlen(clientinfo.storename), 0,
					(SOCKADDR*)&serveraddr, sizeof(serveraddr));
			}
			if (strcmp(buf, "quit") == 0)
				break;
		}
		/*상점 이름 send하고 메뉴 이름 send*/

		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR*)&peeraddr, &addrlen);
		buf[retval] = '\0';
		printf("%s ", buf);

		scanf("%s", clientinfo.order);
		len = strlen(clientinfo.order);
		clientinfo.order[len] = '\0';
		retval = sendto(sock, clientinfo.order, strlen(clientinfo.order), 0,
			(SOCKADDR*)&serveraddr, sizeof(serveraddr));



	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
