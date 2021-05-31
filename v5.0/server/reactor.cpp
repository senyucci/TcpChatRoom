#include "reactor.h"

bool Reactor::InitThreadpool()
{
    if(!InitServer())
    {
        perror("initial server");
        return false;
    }
    //create thread
    pthread_create(&m_accept_tid,NULL,accept_t,(void*)this);
    pthread_create(&m_send_tid,NULL,send_t,(void*)this);
    for(int i = 0 ;i < THREAD_NUM;i++)
    {
        pthread_create(&m_receive_tid[i],NULL,receive_t,(void*)this);
    }
    return true;
}

bool Reactor::InitServer()
{
    // initial server
    m_serverfd = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0);
    if(ERROR == m_serverfd)
    {
        perror("socket");
        return false;
    }
    
    // Port reuse
    int on = 1;
    setsockopt(m_serverfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    setsockopt(m_serverfd, SOL_SOCKET, SO_REUSEPORT, (char *)&on, sizeof(on));


    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(m_serverfd,(sockaddr*)&addr,sizeof(addr)) == ERROR)
    {
        perror("bind");
        return false;
    }

    if(listen(m_serverfd,MAX_CLIENT))
    {
        perror("listen");
        return false;
    }
    cout << "Server is initialized" << endl;

    // initial epoll_fd
    m_epollfd = epoll_create(1);
    if(m_epollfd == ERROR)
    {
        perror("epoll");
        return false;
    }

    struct epoll_event ev;
    bzero(&ev,sizeof(ev));
    ev.events = EPOLLIN | EPOLLHUP;
    ev.data.fd = m_serverfd;
    if(epoll_ctl(m_epollfd,EPOLL_CTL_ADD,m_serverfd,&ev) == ERROR)
    {
        perror("epoll_ctl");
        return false;
    }
    cout << "Epoll-list is initialized" << endl;

    return true;
}

bool Reactor::CloseServer()
{
    m_isStop = true;
    shutdown(m_serverfd,SHUT_RDWR);
    close(m_serverfd);
    close(m_epollfd);
    return true;
}

bool Reactor::CloseClientfd(int fd)
{
    if(epoll_ctl(m_epollfd,EPOLL_CTL_DEL,fd,NULL)==ERROR)
    {
        perror("close c_fd");
        return false;
    }
    close(fd);
    return true;
}

void* Reactor::StartEpoll(void *arg)
{
    cout << "waiting for client's connect" << endl;
    Reactor *pReactor = static_cast<Reactor *>(arg);
    while(!pReactor->m_isStop)
    {
        struct epoll_event ev[EVENT_NUM];
        int res = epoll_wait(pReactor->m_epollfd,ev,EVENT_NUM,-1);
        if(res <= 0)
            continue;
        else
        {
            for(int i = 0;i<res;i++)
            {
                if(ev[i].data.fd == pReactor->m_serverfd)
                    pReactor->m_accept_lock.signal();   // awake accept thread.
                else
                {
                    pReactor->m_recv_lock.lock();
                    pReactor->m_eventclient.push_back(ev[i].data.fd);
                    pReactor->m_recv_lock.unlock();
                    pReactor->m_recv_lock.signal();
                }
            }
        }
    }
    cout <<"Exit epoll..."<<endl;
    return NULL;
}

int Reactor::SetNonBlock(int fd)
{
    int old_opt = fcntl(fd,F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_opt);
    return old_opt;
}

void* Reactor::accept_t(void* arg)
{
    cout << "im accept thread: " << pthread_self()<<endl;
    Reactor *pReactor = (Reactor *)arg;
    while(!pReactor->m_isStop)
    {
        pReactor->m_accept_lock.lock();
        while(pReactor->m_eventclient.empty())
        pReactor->m_accept_lock.wait(); // wait for accept_signal
        
        struct sockaddr_in c_addr;
        socklen_t len;
        bzero(&len,sizeof(len));

        int c_fd = accept(pReactor->m_serverfd,(struct sockaddr*)&c_addr,&len); // accept a new connection
        pReactor->m_accept_lock.unlock();

        if(c_fd == ERROR)
        {
            perror("accept");
            continue;
        }

        c_info c_inf;   // add client's info into set<c_info> m_infos
        c_inf.fd = c_fd;
        c_inf.ip = inet_ntoa(c_addr.sin_addr);
        pReactor->m_temp_lock.lock();
        pReactor->m_infos.insert(c_inf);
        pReactor->m_temp_lock.unlock();

        Reactor::SetNonBlock(c_fd); // set c_fd to nonblock mode

        struct epoll_event ev;  // add c_fd to epoll_list
        ev.events = EPOLLIN|EPOLLRDHUP|EPOLLET;
        ev.data.fd = c_fd;
        if(epoll_ctl(pReactor->m_epollfd,EPOLL_CTL_ADD,c_fd,&ev) == ERROR)
        {
            perror("epoll_add");
            continue;
        }

        cout << c_inf.ip <<" is connected"<<endl;
    }
    return NULL;
}

void* Reactor::receive_t(void* arg)
{
    cout << "im read thread: " << pthread_self()<<endl;
    Reactor* pReactor = (Reactor*)arg;
    while(!pReactor->m_isStop)
    {
        // get a client fd
        int c_fd;
        pReactor->m_recv_lock.lock();
        while(pReactor->m_eventclient.empty())
            pReactor->m_recv_lock.wait();

        c_fd = pReactor->m_eventclient.front();
        pReactor->m_eventclient.pop_front();
        pReactor->m_recv_lock.unlock();
        
        string msg;
        char buff[MAX_BUFFER];
        bool bError = false;
        // get localtime
        time_t now = time(NULL);
        struct tm* str_now = localtime(&now);
        ostringstream str_ostime;

        while(true)
        {
            bzero(buff,sizeof(buff));
            int s_recv = recv(c_fd,buff,MAX_BUFFER,0);
            if(s_recv < 0)     // recv error
            {
                if(errno == EWOULDBLOCK)
                    break;
                else
                {
                    perror("receive");
                    pReactor->CloseClientfd(c_fd);
                    bError = true;
                    break;
                }
            }
            else if(s_recv == 0)    // client closed connect
            {
                pReactor->m_temp_lock.lock();   // erase the c_info
                c_info tmp;
                tmp.fd = c_fd;
                pReactor->m_infos.erase(tmp);   // override operator '<' i
                pReactor->m_temp_lock.unlock();

                

            }
            else    // recv data
            {

            }
        }
    }
    return NULL;
}

void* Reactor::send_t(void* arg)
{
    cout << "im write thread: " << pthread_self()<<endl;
    Reactor* pReactor = (Reactor*)arg;
    while(!pReactor->m_isStop)
    {
        string msg;
        pReactor->m_send_lock.lock();
        while(pReactor->m_msgs.empty())
            pReactor->m_send_lock.wait();
        msg = pReactor->m_msgs.front();
        pReactor->m_msgs.pop_front();
        pReactor->m_send_lock.unlock();
        
        while(true)
        {
            int s_send;
            int c_fd;
            for(auto it = pReactor->m_infos.begin();it != pReactor->m_infos.end();it++)
            {
                c_fd = (*it).fd;
                s_send = send(c_fd,msg.c_str(),msg.length(),0);
                if(s_send == ERROR)
                {
                    if(errno == EWOULDBLOCK)
                    {
                        sleep(10);
                        continue;
                    }
                    else
                    {
                        perror("send");
                        pReactor->CloseClientfd(c_fd);
                        break;
                    }
                }
            }
            msg.clear();
            if(msg.empty())
                break;
        }
    }
    return NULL;
}
