#include "SocketsOps.h"
#include <arpa/inet.h>
#include <cstdio>
#include <strings.h>
#include <sys/socket.h>

#include <muduo/base/Logging.h>
#include <unistd.h>


namespace muduo
{

namespace sockets
{

typedef sockaddr SA;

int createNoblockingOrDie()
{
	int sockFd = ::socket(AF_INET,
			              SOCK_STREAM | 
						  SOCK_CLOEXEC |
						  SOCK_NONBLOCK,
						  0);
	if(sockFd < 0)
	{
		LOG_SYSERR << "sockets::createNoblockingOrDie()";
	}
	return sockFd;
}

void bindOrDie(int sockFd,
		       const struct sockaddr_in& addr)
{
	int ret = ::bind(sockFd, (SA*) &addr, sizeof addr);
	if(ret < 0)
	{
		LOG_SYSERR << "sockets::bindOrDie()";
	}
}

void listenOrDie(int sockFd)
{
	int ret = ::listen(sockFd, SOMAXCONN);

	if(ret < 0)
	{
		LOG_SYSERR << "sockets::listenOrDie()";
	}
}

int accept(int sockFd, struct sockaddr_in* addr)
{
	socklen_t addrLen = sizeof(&addr);
	int connFd = ::accept4(sockFd, 
			               (SA*) addr, 
						   &addrLen,
						   SOCK_CLOEXEC | SOCK_NONBLOCK);
	if(connFd < 0)
	{
		//FIXME
	}
	return connFd;
}

void close(int sockFd)
{
	if(::close(sockFd) == -1)
	{
		LOG_SYSERR << "sockets::close()";
	}
}

void toHostPort(char* buf,
		        size_t size,
				const struct sockaddr_in& addr)
{
	char host[INET_ADDRSTRLEN] = "INVALID";
	::inet_ntop(AF_INET, &(addr.sin_addr), host, sizeof host);
	uint16_t port = networkToHost16(addr.sin_port);
	::snprintf(buf, size, "%s:%u", host, port);
}

void fromHostPort(const char *ip,
		          uint16_t port,
				  struct sockaddr_in* addr)
{
	addr->sin_family = AF_INET;
	addr->sin_port = port;
	if(::inet_pton(AF_INET, ip, &(addr->sin_addr)) <= 0)
	{
		LOG_SYSERR << "sockets::fromHostPort";
	}
}
}//sockets
}//muduo
