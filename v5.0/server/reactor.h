#ifndef _REACTOR_H_
#define _REACTOR_H_
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <time.h>
//#include <mysql.h>
#include <iomanip>
#include <sstream>
#include <list>
#include <set>
#include <deque>
#include <vector>
#include "locker.h"
#include "command.h"

#define ERROR -1
#define PORT 8888
#define THREAD_NUM 5
#define MAX_CLIENT 20
#define EVENT_NUM 1024
#define MAX_BUFFER 2048

using namespace std;

struct c_info;

class Reactor
{
	public:
		Reactor(){}
		~Reactor(){}
		bool InitThreadpool();
		bool InitServer();
		bool CloseServer();
		bool CloseClientfd(int fd);
		static void* StartEpoll(void* arg);
		static int SetNonBlock(int fd);

		private:
		// overload copy_constracter
		Reactor(const Reactor &rhs);
		Reactor &operator=(const Reactor &rhs);

		// thread_function
		static void *accept_t(void *arg);
		static void *receive_t(void *arg);
		static void *send_t(void *arg);

		bool m_isStop = false;
		int m_serverfd = 0;
		int m_epollfd = 0;

		// thread_id
		pthread_t m_accept_tid;
		pthread_t m_send_tid;
		pthread_t m_receive_tid[THREAD_NUM];

		// thread_lock
		Locker m_accept_lock;
		Locker m_recv_lock;
		Locker m_send_lock;
		Locker m_temp_lock;

		// storage data-struct
		set<c_info> m_infos;	// store client info
		deque<string> m_msgs;	// store msg
		list<int> m_eventclient;	// store active chlient
};

struct c_info
{
	int fd;
	string ip;
	string name;
	// overload the '<'
	bool operator<(c_info info)const
	{
		return fd < info.fd;
	}
};

#endif
