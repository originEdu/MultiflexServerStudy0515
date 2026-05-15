#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32")

int main()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//소켓생성
	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

	//리슨서버주소설정
	SOCKADDR_IN ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	ServerSockAddr.sin_family = AF_INET;
	ServerSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerSockAddr.sin_port = htons(43000);

	connect(ServerSocket,(SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));

	while(true)
	{
		char Message[1024] = "메시지 보내기";
		std::string input;
		std::getline(std::cin, input);
		int SentBytes = send(ServerSocket, input.c_str(), input.length(), 0);
		//int SentBytes = send(ServerSocket, Message, (int)strlen(Message), 0);
		if (SentBytes<=0)
		{
			return 0;
		}
		//char RecvMessage[1024] = {0};
		//int RecvBytes = recv(ServerSocket, RecvMessage, sizeof(RecvMessage), 0);
		//printf("RecvMessage : %s\n", RecvMessage);
	}
	closesocket(ServerSocket);
	WSACleanup();
	return 0;
}