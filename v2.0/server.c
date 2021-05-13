#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define PORT 8888
#define SOCKET_ERROR -1
#define MAX_NUM_CLIENT 50
#define MAX_NOTE_SIZE 100
#define MAX_BUFF_SIZE 1024
#define MAX_MSG_SIZE 20 + MAX_BUFF_SIZE
typedef struct
{
	char name[20];
	char *ip;
	int fd;
} Client;

Client client[MAX_NUM_CLIENT];
pthread_t pid[MAX_NUM_CLIENT];
int server_fd;
int num_online = 0;

void ServerInit();
void StartServer();
void *Contact(void *arg);
void SendmsgToALL(char *msg, int fd);
void Clearbuf(char *msg, int len);

int main()
{
	ServerInit();
	StartServer();
	close(server_fd);
	return 0;
}

void ServerInit()
{
	printf("Initializing...\n");
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == server_fd)
		perror("socket"), exit(-1);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int r = bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
	if (-1 == r)
		perror("bind"), exit(-1);
	listen(server_fd, MAX_NUM_CLIENT);
	printf("Server is initialized.\n");
}

void StartServer()
{
	printf("Waiting for client.\n");
	for (;;)
	{
		int flag = 0;
		struct sockaddr_in cAddr;
		socklen_t len = sizeof(cAddr);

		int c_fd = accept(server_fd, (struct sockaddr *)&cAddr, &len);
		if (SOCKET_ERROR == c_fd)
			perror("accept"), exit(-1);
		printf("%s is connected.\n", inet_ntoa(cAddr.sin_addr));

		for (int i = 0; i < MAX_NUM_CLIENT; i++)
		{
			if (client[num_online % MAX_NUM_CLIENT].fd == 0)
			{
				num_online = num_online % MAX_NUM_CLIENT;
				client[num_online].fd = c_fd;
				flag = 1;
				break;
			}
		}
		if (flag = 0)
		{
			char msg[] = "Client number is limited.\n";
			send(c_fd, msg, sizeof(msg), 0);
		}

		pthread_create(&pid[num_online], 0, Contact, (void *)&(client[num_online].fd));
	}
}

void *Contact(void *arg)
{
	int sock_fd = *(int *)arg;
	int temp = num_online++;
	char note[MAX_NOTE_SIZE];
	char buff[MAX_BUFF_SIZE];
	char msg[MAX_MSG_SIZE];
	char name[20];

	if (read(client[temp].fd, name, sizeof(name)) > 0) // recv a name.
	{
		strcpy(client[temp].name, name);
	}

	sprintf(note, "%s is connected.\n", name);
	printf("%s\n", note);
	SendmsgToALL(note, sock_fd);

	for (;;)
	{
		if (read(sock_fd, buff, sizeof(buff)) > 0)
		{
			sprintf(msg, "%s: %s", name, buff);
			printf("%s\n", msg);
			SendmsgToALL(msg, sock_fd);
			Clearbuf(buff, sizeof(buff));
		}
		else
		{
			Clearbuf(note, sizeof(note));
			sprintf(note, "%s is disconnected.\n", name);
			printf("%s\n", note);
			SendmsgToALL(note, sock_fd);
			
			for (int i = 0; i < num_online; i++) //set offline fd to 0.
			{
				if (client[i].fd == sock_fd)
					client[i].fd = 0;
			}
			pthread_exit(NULL);
		}
	}
}

void SendmsgToALL(char *msg, int fd)
{
	for (int i = 0; i < num_online; i++)
	{
		if (client[i].fd != 0 && client[i].fd != fd)
		{
			send(client[i].fd, msg, strlen(msg), 0);
		}
	}
}

void Clearbuf(char *msg, int len)
{
	memset(msg, 0, len);
}
