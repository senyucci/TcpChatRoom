#include "reactor.h"

Reactor::~Reactor()
{
    CloseServer();
}

bool Reactor::InitServer()
{
    // create server
    m_serverfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (ERROR == m_serverfd)
    {
        perror("socket");
        return false;
    }

    // Port reuse
    int on = 1;
    setsockopt(m_serverfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    setsockopt(m_serverfd, SOL_SOCKET, SO_REUSEPORT, (char *)&on, sizeof(on));

    // initial server
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(m_serverfd, (sockaddr *)&addr, sizeof(addr)) == ERROR)
    {
        perror("bind");
        return false;
    }

    if (listen(m_serverfd, MAX_CLIENT))
    {
        perror("listen");
        return false;
    }
    cout << "Server is initialized" << endl;

    // initial epoll_fd
    m_epollfd = epoll_create(1);
    if (m_epollfd == ERROR)
    {
        perror("epoll");
        return false;
    }

    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.events = EPOLLIN | EPOLLHUP;
    ev.data.fd = m_serverfd;
    if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_serverfd, &ev) == ERROR)
    {
        perror("epoll_ctl");
        return false;
    }
    cout << "Epoll-list is initialized" << endl;

    ARG *arg = new ARG();
    arg->pThis = this;
    // start threadpool
    pthread_create(&m_accept_tid, NULL, accept_t, (void *)arg);
    pthread_create(&m_send_tid, NULL, send_t, (void *)arg);
    for (int i = 0; i < THREAD_NUM; i++)
    {
        pthread_create(&m_receive_tid[i], NULL, receive_t, (void *)arg);
    }
    cout << "ThreadPoll is initialized" << endl;
    return true;
}

bool Reactor::CloseServer()
{
    m_isStop = true;
    shutdown(m_serverfd, SHUT_RDWR);
    close(m_serverfd);
    close(m_epollfd);
    return true;
}

bool Reactor::CloseClientfd(int fd)
{
    if (epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, NULL) == ERROR)
    {
        perror("close c_fd");
        return false;
    }
    close(fd);
    return true;
}

void *Reactor::StartEpoll(void *arg)
{
    cout << "waiting for client's connect" << endl;
    Reactor *pReactor = static_cast<Reactor *>(arg);

    while (!pReactor->m_isStop)
    {
        struct epoll_event ev[EVENT_NUM];
        int res = epoll_wait(pReactor->m_epollfd, ev, EVENT_NUM, 10);
        if (res <= 0)
            continue;

        for (int i = 0; i < res; i++)
        {
            if (ev[i].data.fd == pReactor->m_serverfd)
            {
                pReactor->m_accept_lock.signal(); // awake accept thread.
            }
            else
            {
                pReactor->m_recv_lock.lock();
                pReactor->m_eventclient.push_back(ev[i].data.fd);
                pReactor->m_recv_lock.unlock();
                pReactor->m_recv_lock.signal(); // awake read thread.
            }
        }
    }
    cout << "Exit epoll..." << endl;
    return NULL;
}

int Reactor::SetNonBlock(int fd) // set nonblocking.
{
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opt);
    return old_opt;
}

void *Reactor::accept_t(void *arg)
{
    ARG *args = (ARG *)arg;
    Reactor *pReactor = args->pThis;
    cout << "im accept thread: " << pthread_self() << endl;
    while (!pReactor->m_isStop) // a circle hold this thread alive
    {
        // wait for accept_signal
        pReactor->m_accept_lock.lock();
        pReactor->m_accept_lock.wait();

        c_info c_inf;
        struct sockaddr_in c_addr;
        socklen_t len;
        bzero(&len, sizeof(len));

        // accept a new connection
        int c_fd = accept(pReactor->m_serverfd, (struct sockaddr *)&c_addr, &len);
        pReactor->m_accept_lock.unlock();
        if (c_fd == ERROR)
            continue;

        // add client's info into m_infos (a set store client infos)
        c_inf.fd = c_fd;
        c_inf.ip = Reactor::GetIpAddr(c_fd);
        pReactor->m_infos_lock.lock();
        pReactor->m_infos.insert(c_inf);
        pReactor->m_infos_lock.unlock();

        // set c_fd to nonblock mode
        Reactor::SetNonBlock(c_fd);

        // add c_fd to epoll_list
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
        ev.data.fd = c_fd;
        if (epoll_ctl(pReactor->m_epollfd, EPOLL_CTL_ADD, c_fd, &ev) == ERROR)
        {
            perror("epoll_add");
            continue;
        }

        cout << c_inf.ip << " is Connected" << endl;
    }
    return NULL;
}

    void *Reactor::receive_t(void *arg)
    {
        ARG *args = (ARG *)arg;
        Reactor *pReactor = args->pThis;
        cout << "im read thread: " << pthread_self() << endl;

        while (!pReactor->m_isStop)
        {
            // get an active client fd
            int c_fd;
            c_info c_tmp;
            const c_info *pC_info; // client information
            string msg;
            char buff[MAX_BUFFER];
            bool bError = false;

            // get localtime
            time_t now = time(NULL);
            struct tm *str_now = localtime(&now);
            ostringstream str_ostime;

            pReactor->m_recv_lock.lock();
            while (pReactor->m_eventclient.empty())
            {
                pReactor->m_recv_lock.wait();
            }
            c_fd = pReactor->m_eventclient.front();
            pReactor->m_eventclient.pop_front();

            //use set's iterator traverse all elements to get client information
            for (auto it = pReactor->m_infos.begin(); it != pReactor->m_infos.end(); it++)
            {
                if ((*it).fd == c_fd)
                {
                    pC_info = &(*it);
                    break;
                }
            }
            pReactor->m_recv_lock.unlock();

            while (true) // handler
            {
                bzero(buff, sizeof(buff));
                int s_recv = recv(c_fd, buff, MAX_BUFFER, 0);
                if (s_recv < 0) // recv error
                {
                    if (errno == EWOULDBLOCK)
                        break;
                }
                else if (s_recv == 0) // client disconnected
                {
                    pReactor->m_infos_lock.lock(); // erase the c_info
                    msg = pC_info->name + " is Disconnected\n";
                    cout << msg;
                    msg.clear();
                    pReactor->m_infos.erase(*pC_info); // need to override operator '<'
                    pReactor->m_infos_lock.unlock();

                    pReactor->m_msg_lock.lock();
                    pReactor->m_msgs.push_back(msg);
                    pReactor->m_msg_lock.unlock();
                    pReactor->m_send_lock.signal();

                    pReactor->CloseClientfd(c_fd);
                }
                else // recv data
                {
                    string strtmp = buff;
                    msg = strtmp.substr(1, strtmp.length());

                    if (buff[0] == '1')
                    {
                        pReactor->m_infos_lock.lock();

                        c_tmp.fd = c_fd;
                        pReactor->m_infos.erase(c_tmp);
                        c_tmp.name = msg;
                        c_tmp.ip = pC_info->ip;
                        pReactor->m_infos.insert(c_tmp);

                        pReactor->m_infos_lock.unlock();
                        msg.clear();
                    }
                    else if (buff[0] == '0')
                    {
                        pReactor->m_infos_lock.lock();
                        msg = pC_info->name + ": " + msg;
                        pReactor->m_infos_lock.unlock();

                        cout << msg << endl;

                        pReactor->m_msg_lock.lock();
                        pReactor->m_msgs.push_back(msg);
                        pReactor->m_msg_lock.unlock();
                        pReactor->m_send_lock.signal();
                    }
                    else
                    {
                        cout << "error in recv_t L:279" << endl;
                    }
                }
            }
        }
        return NULL;
    }

    void *Reactor::send_t(void *arg)
    {
        ARG *args = (ARG *)arg;
        Reactor *pReactor = args->pThis;
        cout << "im write thread: " << pthread_self() << endl;

        while (!pReactor->m_isStop)
        {
            string msg;
            pReactor->m_send_lock.lock();
            while (pReactor->m_msgs.empty())
            {
                pReactor->m_send_lock.wait();
            }
            msg = pReactor->m_msgs.front();
            pReactor->m_msgs.pop_front();
            pReactor->m_send_lock.unlock();

            while (true)
            {
                int s_send;
                c_info c_inf;
                pReactor->m_infos_lock.lock();
                for (auto it = pReactor->m_infos.begin(); it != pReactor->m_infos.end(); it++)
                {
                    c_inf = (*it);
                    s_send = send(c_inf.fd, msg.c_str(), msg.length(), 0);
                    if (s_send == ERROR)
                    {
                        if (errno == EWOULDBLOCK)
                            continue;
                        else
                        {
                            perror("send");
                            pReactor->CloseClientfd(c_inf.fd);
                            break;
                        }
                    }
                }
                pReactor->m_infos_lock.unlock();

                msg.clear();
                if (msg.empty())
                    break;
            }
        }
        return NULL;
    }

    string Reactor::GetIpAddr(int fd)
    {
        char ip[20];
        struct sockaddr_storage addr;
        socklen_t len = sizeof(addr);

        // get fd's IP address
        getpeername(fd, (struct sockaddr *)&addr, &len);
        struct sockaddr_in *in = (struct sockaddr_in *)&addr;

        inet_ntop(AF_INET, &in->sin_addr, ip, sizeof(ip));
        string str_ip = ip;
        return str_ip;
    }