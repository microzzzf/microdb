#include "networker.h"

using namespace microdb;

int connected = 0;

void reset(int)
{
    connected = 0;
}

bool Networker::Listener::init()
{
    if ((fd_ = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        std::cout<<strerror(errno)<<std::endl;
        return false;
    }   

    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port_);
    addr_.sin_addr.s_addr = INADDR_ANY;
    bzero(&(addr_.sin_zero), 8);

    if (bind(fd_, (struct sockaddr *)(&addr_), sizeof(sockaddr)) == -1)
    {
        std::cout<<strerror(errno)<<std::endl;
        return false;
    }

    if (listen(fd_, backlog_) == -1)
    {
        std::cout<<strerror(errno)<<std::endl;
        return false;
    }

    return true;
}

Networker::Listener::~Listener()
{
    if (fd_ > -1)
    {
        close(fd_);
    }
}

void Networker::Connector::work(int listener)
{
    int len = 0;
    socklen_t sin_size = sizeof(sockaddr_in);
    while (1)
    {
        if ((fd_ = accept(listener, (sockaddr *)(&addr_), &sin_size)) == -1)
        {   
            std::cout<<strerror(errno)<<std::endl;
            continue;
        }
        std::cout<<"Connected to a new client, fd "<<fd_<<std::endl;
        connected = true;
        while (connected)
        {
            len = recv(fd_, buffer_, sizeof(buffer_), 0);   
            std::cout<<"Received buffer: "<<buffer_;
            memset(buffer_, 0, sizeof(buffer_));    
            memcpy(buffer_, "done\n", 5);
            send(fd_, buffer_, strlen(buffer_), 0);
            signal(SIGPIPE, reset);
            //connected_ = false;
            memset(buffer_, 0, sizeof(buffer_));
        }
    }   
}

void Networker::Connector::start(int listener)
{
    thread_ = new std::thread(std::mem_fn(&Networker::Connector::work), this, listener);
}

void Networker::Connector::stop()
{
    if (thread_)
    {
        thread_->join();
    }
}

Networker::Connector::~Connector()
{
    if (fd_ > -1)
    {
        close(fd_);
    }

    if (thread_)
    {
        delete thread_;
        thread_ = nullptr;
    }
}

void Networker::EpollConnector::start(int listener)
{
    struct epoll_event event, events[MAXEVENTS];

    fd_ = epoll_create(MAXCONNECTORS);

    event.data.fd = listener;
    event.events=EPOLLIN|EPOLLET;
    epoll_ctl(fd_, EPOLL_CTL_ADD, listener, &event);

    int connect_fd = -1, sock_fd = -1;
    struct sockaddr_in addr;
    socklen_t len = sizeof(sockaddr_in);

    int nfds = 0, n = -1;
    while (1)
    {
        nfds=epoll_wait(fd_, events, MAXEVENTS, MAXCONNECTORS);
        for (auto i = 0; i < nfds; ++i)
        {
            if(events[i].data.fd == listener)
            {
                if ((connect_fd = accept(listener, (sockaddr*)(&addr), &len)) < 0)
                {
                    std::cout<<"Connection fd can't be less than zero!"<<std::endl;
                    return;         
                }
                event.data.fd = connect_fd;
                event.events=EPOLLIN|EPOLLET;
                epoll_ctl(fd_, EPOLL_CTL_ADD, connect_fd,&event);
            }
            else if (events[i].events&EPOLLIN)
            {
                if ((sock_fd = events[i].data.fd) < 0)
                {
                    continue;
                }
                if ((n = read(sock_fd, buffer_, sizeof(buffer_))) < 0) 
                {
                    if (errno == ECONNRESET) 
                    {
                                    close(sock_fd);
                                    events[i].data.fd = -1;
                                }
                    else
                    {
                                    std::cout<<"Epoll read error!"<<std::endl;
                    }
                }
                else if (n == 0)
                {
                    close(sock_fd);
                                events[i].data.fd = -1;
                }
                buffer_[n] = '\0';
                std::cout<<"Epoll receive buffer: "<<buffer_;
                event.data.fd=sock_fd;
                event.events=EPOLLOUT|EPOLLET;
                epoll_ctl(fd_, EPOLL_CTL_MOD, sock_fd, &event);
            }
            else if(events[i].events&EPOLLOUT)
            {
                sock_fd = events[i].data.fd;
                memcpy(buffer_, "done\n\0", 6);
                        write(sock_fd, buffer_, n);
                                event.data.fd=sock_fd;
                                event.events=EPOLLIN|EPOLLET;
                                epoll_ctl(fd_, EPOLL_CTL_MOD, sock_fd, &event);
            }
        }
    }
}

Networker::EpollConnector::~EpollConnector()
{
    if (fd_ > -1)
    {
        close(fd_);
    }
}

Networker::Networker(int port_in, int backlog_in, Networker::MODE mode_in)
{
    mode_ = mode_in;

    listener_ = new Listener(port_in, backlog_in);

    Connector* conn = nullptr;

    switch (mode_)
    {
    case Networker::BLOCKING:
        for (auto i = 0; i < MAXCONNECTORS; ++i)
        {
            conn = new Connector();
            connectors_.push_back(conn);
        }
        break;
    case Networker::EPOLL:
        epoll_connector_ = new EpollConnector();
        break;
    default:
        break;
    }
}

Networker::~Networker()
{
    if (listener_)
    {
        delete listener_;
        listener_ = nullptr;
    }
    
    switch (mode_)
    {
    case Networker::BLOCKING:
        for (auto i = 0; i < connectors_.size(); ++i)
        {
            if (connectors_[i])
            {
                delete connectors_[i];
                connectors_[i] = nullptr;
            }
        }
        break;
    case Networker::EPOLL:
        if (epoll_connector_)
        {
            delete epoll_connector_;
            epoll_connector_ = nullptr;
        }
        break;
    default:
        break;
    }
}

Networker* Networker::getInstance()
{
    static Networker networker(3333, 10, Networker::BLOCKING);
    return &networker;
}

bool Networker::work()
{
    if (!listener_)
    {
        std::cout<<"Listener can't be nullptr!"<<std::endl;
        return false;
    }

    if (listener_->init() == false)
    {
        std::cout<<"Init listener failed!"<<std::endl;
        return false;
    }
    
    switch (mode_)
    {
    case Networker::BLOCKING:
        for (auto i = 0; i < connectors_.size(); ++i)
        {
            if (!connectors_[i])
            {
                std::cout<<"Connector can't be nullptr"<<std::endl;
                return false;
            }
            connectors_[i]->start(listener_->getFd());
        }   

        for (auto i = 0; i < connectors_.size(); ++i)
        {
            connectors_[i]->stop();
        }
        break;
    case Networker::EPOLL:
        epoll_connector_->start(listener_->getFd());
        break;
    default:
        break;
    }

    return true;
}
