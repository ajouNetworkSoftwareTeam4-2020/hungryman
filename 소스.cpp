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
// ���� �Լ� ���� ��� �� ����
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

int main(int argc, char* argv[])
{
	int retval;
	infotoClient clientinfo;
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// ���� �ּ� ����ü �ʱ�ȭ
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8000);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// ������ ��ſ� ����� ����
	SOCKADDR_IN peeraddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	int len;

	// ������ ������ ���
	while (1) {
		// ������ �Է�


		/*ID �Է�*/
		printf("ID �Է� : ");
		scanf("%s", clientinfo.id);
		len = strlen(clientinfo.id);
		clientinfo.id[len] = '\0';

		retval = sendto(sock, clientinfo.id, strlen(clientinfo.id), 0,
			(SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}

		/*PW �Է�*/
		printf("PW �Է� : ");
		scanf("%s", clientinfo.pw);
		len = strlen(clientinfo.pw);
		clientinfo.pw[len] = '\0';
		retval = sendto(sock, clientinfo.pw, strlen(clientinfo.pw), 0,
			(SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}


		/*ī�װ� ���� �ޱ�*/
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR*)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}
		buf[retval] = '\0';
		printf("%s ", buf);

		/*ī�װ� �����ؼ� send������ */
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
		/*���� ���� �ް� ����ϱ�*/
		//while���� send ���� �� ���� ���ƾ� �Ѵ�. -> ���� : ��Ʈ ���� �� �ߴ� �� �� �ֵ��� �ߴ�.
		while (1) {
			retval = recvfrom(sock, buf, BUFSIZE, 0,
				(SOCKADDR*)&peeraddr, &addrlen);
			if (retval == SOCKET_ERROR) {
				err_display("recvfrom()");
				continue;
			}
			buf[retval] = '\0';
			printf("%s", buf);
			if (strcmp(buf, "�����̸� : ") == 0) {
				scanf("%s", clientinfo.storename);
				len = strlen(clientinfo.storename);
				clientinfo.storename[len] = '\0';
				retval = sendto(sock, clientinfo.storename, strlen(clientinfo.storename), 0,
					(SOCKADDR*)&serveraddr, sizeof(serveraddr));
			}
			if (strcmp(buf, "quit") == 0)
				break;
		}
		/*���� �̸� send�ϰ� �޴� �̸� send*/

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

	// ���� ����
	WSACleanup();
	return 0;
}
