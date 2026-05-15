#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32")

int main()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//소켓생성
	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, 0);

	//리슨서버주소설정
	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ListenSockAddr.sin_port = htons(43000);

	//바인드
	bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));
	//리슨
	listen(ListenSocket, 5);

	//select 초기설정
	int ReadSocketCount = 0; //windows X
	fd_set ReadSockets; //집합선언
	fd_set CopyReadSockets; //select인자에 넣을 복사본
	FD_ZERO(&ReadSockets); //초기화
	FD_SET(ListenSocket, &ReadSockets); //(감시할녀석, 등록할 fdset)
	//-> 리슨 소켓 감시해달라고 한거임

	//accept클라주소 설정
	SOCKADDR_IN ClientSockAddr;
	memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
	int ClientSockAddrLength = sizeof(ClientSockAddr);

	//select TimeOut 설정
	struct timeval TimeOut;
	TimeOut.tv_sec = 0; // 0초
	TimeOut.tv_usec = 100000; // 0.1초, 단위가 마이크로초임
	while (true)
	{
		CopyReadSockets = ReadSockets;
		int ChangeSocketCount = select(0, &CopyReadSockets, 0, 0, &TimeOut);
		//바뀐게 몇개인지 반환해줌 -> 문제는 어떤 녀석이 바뀌었는지 안알려줌
		//select는 &CopyReadSockets의 값을 바꿔서준다
		if (ChangeSocketCount <= 0) //변화된 내용이 없음
		{
			continue;
		}
		
		//소켓 처리 recv,accept
		//변화된 내용이 있으면 원본소켓과 Copy소켓과 비교한다
		//무식하게 전부
		for (int i = 0; i < (int)ReadSockets.fd_count; ++i)
		{
			//변경된점 있냐?
			//현재 소켓이 CopyReadSockets에 남아있는지 확인
			//남아있으면 변경된 것임
			if (FD_ISSET(ReadSockets.fd_array[i], &CopyReadSockets))
			{
				//그거 리슨이냐?
				if (ReadSockets.fd_array[i] == ListenSocket)
				{
					//맞으면 accept(새로운 클라이언트 접속 처리)
					SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSockAddr, &ClientSockAddrLength);
					printf("클라연결됨\n");
					FD_SET(ClientSocket,&ReadSockets); //클라소켓도 감시해주세요
				}
				else
				{
					//아니면 자료구나 recv
					char Buffer[1024] = { 0 };
					int RecvBytes = recv(ReadSockets.fd_array[i], Buffer, sizeof(Buffer), 0);
					if (RecvBytes<=0) //접속이 끊겼거나 네트워크오류
					{
						//소켓닫고 ReadSockets에서 소켓 제거
						closesocket(ReadSockets.fd_array[i]);
						FD_CLR(ReadSockets.fd_array[i],&ReadSockets);
						printf("클라종료됨\n");
					}
					else //메세지가 온거임
					{
						printf("%s\n", Buffer);
						//받은 메시지 모두에게 전송
						for (int j = 0; j < (int)ReadSockets.fd_count; ++j)
						{
							if (ReadSockets.fd_array[j] != ListenSocket)
							{
								int SendBytes = send(ReadSockets.fd_array[j], Buffer, (int)strlen(Buffer), 0);
								if (SendBytes<=0)
								{
									closesocket(ReadSockets.fd_array[j]);
									FD_CLR(ReadSockets.fd_array[j], &ReadSockets);
									printf("클라종료됨\n");
									j--;
								}
							}
						}
					}
				}
			}
		}
	}

	closesocket(ListenSocket);
	WSACleanup();
	return 0;
}