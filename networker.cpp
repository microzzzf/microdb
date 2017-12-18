#include "networker.h"

using namespace microdb;

void reset(int)
{

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
		connected_ = true;
		while (connected_)
		{
			len = recv(fd_, buffer_, sizeof(buffer_), 0);	
			std::cout<<"Received buffer: "<<buffer_;
			memset(buffer_, 0, sizeof(buffer_));	
			memcpy(buffer_, "done\n", 5);
			send(fd_, buffer_, strlen(buffer_), 0);
			signal(SIGPIPE, reset);
			connected_ = false;
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

Networker::Networker(int port_in, int backlog_in)
{
	listener_ = new Listener(port_in, backlog_in);
	Connector* conn = nullptr;
	for (auto i = 0; i < MAXCONNECTORS; ++i)
	{
		conn = new Connector();
		connectors_.push_back(conn);
	}
}

Networker::~Networker()
{
	if (listener_)
	{
		delete listener_;
		listener_ = nullptr;
	}

	for (auto i = 0; i < connectors_.size(); ++i)
	{
		if (connectors_[i])
		{
			delete connectors_[i];
			connectors_[i] = nullptr;
		}
	}
}

Networker* Networker::getInstance()
{
	static Networker networker(3333, 10);
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

	return true;
}
