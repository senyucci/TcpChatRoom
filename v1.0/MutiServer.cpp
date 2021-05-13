#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<iostream>
#include<WinSock2.h>
using namespace std;

#pragma comment(lib,"ws2_32.lib")
#define PORT 8888
#define MAX_NUM_CLIENT 30
#define MAX_BUFFSIZE 2048

SOCKET clientSocket[MAX_NUM_CLIENT];

void Threadrun(int i);

int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "Failed to confirm protocol version." << endl;
		return -1;
	}
	cout << "Confirmed protocol-version." << endl;


	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SOCKET_ERROR == serverSocket)
	{
		cout << "Failed to create socket: " << GetLastError() << endl;
		WSACleanup();
		return -1;
	}
	cout << "Created Socket." << endl;
	cout << "Connecting to Server..." << endl;


	SOCKADDR_IN addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORT);
	cout << "Connected." << endl;


	int r = bind(serverSocket, (sockaddr*)&addr, sizeof addr);
	if (-1 == r)
	{
		cout << "Failed to bind Socket: " << GetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return -1;
	}
	cout << "Binded Socket." << endl;



	r = listen(serverSocket, MAX_NUM_CLIENT);
	if (-1 == r)
	{
		cout << "Failed to listen Socket: " << GetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return -1;
	}
	cout << "Listen to Socket." << endl;


	SOCKADDR_IN cAddr = { 0 };
	int len = sizeof cAddr;

	for (int i = 0; i < MAX_NUM_CLIENT; i++)
	{
		clientSocket[i] = accept(serverSocket, (sockaddr*)&cAddr, &len);
		if (SOCKET_ERROR == clientSocket[i])
		{
			cout << "Server Error: " << GetLastError() << endl;
			closesocket(serverSocket);
			WSACleanup();
			return -1;
		}
		cout << "Client " << i + 1 << " connect success: " << inet_ntoa(cAddr.sin_addr) << endl;

		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Threadrun, (LPVOID)i, NULL, NULL);
	}


	closesocket(serverSocket);

	WSACleanup();


	return 0;
}

void Threadrun(int i)
{
	char buff[MAX_BUFFSIZE];
	char name[20];

	memset(name, 0, 20);
	int bitSize = recv(clientSocket[i], name, 20, 0); // recv name.
	while (1)	// waiting Client's data.
	{
		bitSize = recv(clientSocket[i], buff, MAX_BUFFSIZE - 1, NULL);
		if (bitSize > 0)
		{
			buff[bitSize] = 0;
			cout << name << ": " << buff << endl;
			for (int j = 0; j < MAX_NUM_CLIENT; j++)
			{
				if (j == i)
					continue;
				if (!(SOCKET_ERROR == clientSocket[j]))
				{
					fflush(NULL);
					send(clientSocket[j], buff, strlen(buff), NULL);
				}
			}
		}
	}
}