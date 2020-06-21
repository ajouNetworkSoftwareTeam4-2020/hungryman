// UDPEchoClient.c
// UDP echo client
//
// �����: ��Ʈ��ũ����Ʈ������
// ���ִ��б� ����Ʈ�����а�
// �̵� ��Ƽ�̵�� ���� ��Ʈ��ũ ������ (mmcn.ajou.ac.kr)
//
#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 512
#define PORT 8400

typedef struct adminprotocol {
	char id[40];		//ȸ�� ���� �̸�
	char name[40];		//���̵�.   ���н� ������ ����� ����
	char password[40];	//��й�ȣ
	int result;		// ���� �� ����. (���ڸ� ok 0�̸� ����)
	int howdo;		//�α��� / ȸ�� ���� ����, (1�� �α���)
}AdminProtocol;

// ���� �Լ� ���� ��� �� ����
void err_quit(char* msg)
{
	printf("%s\n", msg);
}

// ���� �Լ� ���� ���
void err_display(char* msg)
{
	printf("%s\n", msg);
}

int identity;

int main(int argc, char* argv[])
{
	int retval;

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
	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// ������ ��ſ� ����� ����
	SOCKADDR_IN peeraddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	int len;

	printf("*************************************\n");
	printf("*   Rider Client !!					*\n");
	printf("*************************************\n\n");

	// ������ ������ ���
	while (1) {
		int direction = 0;
		printf("���� �� �۾��� �����ϼ��� \n");
		printf("1. �α���\n");
		printf("2. ȸ�� ����\n");
		scanf("%d", &direction);

		AdminProtocol* request = (AdminProtocol*)malloc(sizeof(AdminProtocol));
		printf("���̵� �Է��ϼ���\n");
		scanf("%s", request->id);
		request->id[strlen(request->id)] = '\0';
		printf("��� ��ȣ�� �Է��ϼ���\n");
		scanf("%s", request->password);
		request->password[strlen(request->password)] = '\0';

		if(direction == 1) {
			request->howdo = 1;
		}
		else {
			request->howdo = 2;
			printf("�̸��� �Է��ϼ��� \n");
			scanf("%s", request->name);
			request->name[strlen(request->name)] = '\0';
		}

		// ������ ������
		retval = sendto(sock, (char*)request, sizeof(AdminProtocol), 0,
			(SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}
		free(request);

		// ������ �ޱ�
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
				printf("�α��� �Ǿ����ϴ�.");
				identity = response->result;
				break;
			}
			else {
				printf("ȸ�� ���� �Ǿ����ϴ�.");
			}
		}
	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
