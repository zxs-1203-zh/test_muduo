#include "SocketsOps.h"
#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>

#include <muduo/base/Logging.h>


namespace muduo
{

namespace sockets
{

typedef struct sockaddr SA;

int createNonblockOrDie()
{
	int sockFd = ::socket(AF_INET, 
			              SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 
						  IPPROTO_TCP);

	if(sockFd < 0)
	{
		LOG_SYSERR << "muduo::sockets::createNonblockOrDie()";
	}

	return sockFd;
}

void bindOrDie(int sockFd, const struct sockaddr_in& addr)
{
	int ret = ::bind(sockFd, (SA*) &addr, sizeof(addr));

	if(ret == -1)
	{
		LOG_SYSERR << "muduo::sockets::bindOrDie()";
	}
}

void listenOrDie(int sockFd)
{
	int ret = ::listen(sockFd, SOMAXCONN);

	if(ret == -1)
	{
		LOG_SYSERR << "muduo::sockets::listenOrDie()";
	}
}

int accept(int sockFd, struct sockaddr_in* addr)
{
	socklen_t len = sizeof(&addr);

	int connFd = ::accept4(sockFd, (SA*) addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);

	if(connFd == -1)
	{
		//FIXME
	}

	return connFd;
}

void close(int sockFd)
{
	int ret = ::close(sockFd);

	if(ret == -1)
	{
		LOG_SYSERR << "muduo::sockets::close()";
	}
}

void fromHostPort(const char* ip, uint16_t host, struct sockaddr_in* addr)
{
	addr->sin_family = AF_INET;
	addr->sin_port = hostToNetwork16(host);
	if(::inet_pton(AF_INET, ip, (void *) &addr->sin_addr) <= 0)
	{
		LOG_SYSERR << "muduo::sockets::fromHostPort()";
	}
}

void toHostPort(char* buf, size_t size, const struct sockaddr_in& addr)
{
	char ip[INET_ADDRSTRLEN] = "INVALID";
	::inet_ntop(AF_INET, (void*) &addr.sin_addr, ip, sizeof(ip));
	uint16_t port = networkToHost16(addr.sin_port);
	::snprintf(buf, size, "%s:%u", ip, port);
}

struct sockaddr_in getLocalAddr(int sockFd)
{
	struct sockaddr_in localAddr;
	::bzero(&localAddr, sizeof localAddr);
	socklen_t addrLen = sizeof localAddr;
	if(::getsockname(sockFd, (SA*) &localAddr, &addrLen) != 0)
	{
		LOG_SYSERR << "sockets::getLocalAddr";
	}
	return localAddr;
}

int getSocketError(int sockFd)
{
	int optval = 0;
	socklen_t optlen = sizeof optlen;
	if(::getsockopt(sockFd, SOL_SOCKET, SO_ERROR, &optlen, &optlen) < 0)
	{
		return errno;
	}
	else
	{
		return optval;
	}
	assert(false);
}

}//sockets

}//muduo
