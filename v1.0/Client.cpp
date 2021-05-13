#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<iostream>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#define MAXSIZE 1023
#define PORT 8888
using namespace std;

SOCKET CreateSocket();
int Connect(SOCKET cSocket);
void Contact(SOCKET cSocket);
void RecvData(SOCKET cSocket);

SOCKADDR_IN addr;

int main()
{
	SOCKET cSocket = CreateSocket();

	Connect(cSocket);
	Contact(cSocket);

	for (;;);
	closesocket(cSocket);
	WSACleanup();

	return 0;

}

SOCKET CreateSocket()
{
	// 1. 确定协议版本
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "Failed to Confirm version..." << endl;
		return -1;
	}
	cout << "校验协议版本..." << endl;
	cout << "校验成功" << endl;


	// 2. 创建 Socket
	SOCKET cSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SOCKET_ERROR == cSocket)
	{
		cout << "Failed to create socket: " << GetLastError() << endl;
		WSACleanup();
		return -1;
	}
	cout << "创建socket..." << endl;
	cout << "创建完成" << endl;


	// 3. 设置服务器协议地址簇
	memset(&addr, 0, sizeof addr);
	addr.sin_family = PF_INET;	// 需要和 socket 函数的第一个参数一致
	addr.sin_addr.S_un.S_addr = inet_addr("81.69.236.101"); //81.69.236.101//100.130.59.30
	addr.sin_port = htons(PORT);	// 端口号

	return cSocket;
}


int Connect(SOCKET cSocket)
{
	// 4. 连接
	cout << "正在连接服务器..." << endl;
	int r = connect(cSocket, (sockaddr*)&addr, sizeof(addr));
	if (-1 == r)
	{
		cout << "Failed to connect Server: " << GetLastError() << endl;
		closesocket(cSocket);
		WSACleanup();
		system("pause");
		exit(0);
	}
	cout << "服务器已连接" << endl;
	return r;
}

void Contact(SOCKET cSocket)
{
	char buff[MAXSIZE];
	char name[20];
	memset(name, 0, 20);
	
	fflush(stdin);
	cout <<endl<<"输入您的昵称: ";
	cin >> name;
	fflush(stdin);
	send(cSocket, name, 20, NULL);
	cout << "输入: " << endl;

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)RecvData, (LPVOID)cSocket, NULL, NULL);

	// 5. 通信
	for (;;)
	{
		memset(buff, 0, MAXSIZE);
		fflush(stdin);
		cin.getline(buff, MAXSIZE);
		send(cSocket, buff, strlen(buff), 0);
		//cout << "成功发送 " << strlen(buff) << " 字节数据" << endl << endl;
	}
}


void RecvData(SOCKET cSocket)
{
	char buf[MAXSIZE];
	for (;;)
	{
		int bit = recv(cSocket, buf, MAXSIZE - 1, NULL);
		if (bit > 0)
		{
			buf[bit] = 0;
			cout << buf << endl;
		}
		else
		{
			cout << "Disconnected." << endl;
			system("pause");
			exit(0);
		}
	}
}
