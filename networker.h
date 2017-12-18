#ifndef _NETWORKER_H
#define _NETWORKER_H

#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <vector>

namespace microdb
{

class Networker
{
public:
	enum MODE
	{
		BLOCKING,
		EPOLL
	};

	class Listener
	{
	public:
		Listener(int port_in, int backlog_in)
		: fd_(-1)
		, port_(port_in)
		, backlog_(backlog_in)
		{ }

		bool init();
		inline int getFd()
		{
			return fd_;	
		}

		virtual ~Listener();
	private:
		int fd_;
		struct sockaddr_in addr_;
		int port_; 
		int backlog_;
	};

	class Connector
	{
	public:
		Connector()
		: fd_(-1)
		, connected_(false)
		{
			memset(buffer_, 0, sizeof(buffer_)); 
		}

		void start(int listener);	
		void stop();

		virtual ~Connector();
	private:
		void work(int listener);

		static const int BUFFERLEN = 4096;
		char buffer_[BUFFERLEN];
		int fd_;	
		struct sockaddr_in addr_;
		std::thread* thread_;
		bool connected_;
	};

	class EpollConnector
	{
	public:
		EpollConnector()
		: fd_(-1)
		{ }

		void start(int listener);

		virtual ~EpollConnector();
	private:
		static const int BUFFERLEN = 4096;
		int fd_;
		char buffer_[BUFFERLEN];
		static const int MAXEVENTS = 20;
	};

	static Networker* getInstance();
	bool work();
private:
	Networker(int port_in, int backlog_in, MODE mode_in);
	virtual ~Networker();

	Listener* listener_;
	std::vector<Connector*> connectors_;
	EpollConnector* epoll_connector_;
	MODE mode_;

	static const int MAXCONNECTORS = 3;
};

} // namespace microdb

#endif // _NETWORKER_H
