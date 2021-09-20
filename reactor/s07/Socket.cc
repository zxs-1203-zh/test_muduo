#include "InetAddress.h"
#include "SocketsOps.h"
#include "Socket.h"

#include <asm-generic/socket.h>
#include <cstring>
#include <sys/socket.h>

#include <muduo/base/Logging.h>

using namespace muduo;

Socket::~Socket()
{
	sockets::close(sockFd_);
}

void Socket::bind(const InetAddress& addr)
{
	sockets::bindOrDie(sockFd_, addr.getSockAddrInet());
}

void Socket::listen()
{
	sockets::listenOrDie(sockFd_);
}

int Socket::accept(InetAddress* peerAddr)
{
	struct sockaddr_in addr;
	::bzero(&addr, sizeof(addr));
	int connFd = sockets::accept(sockFd_, &addr);
	peerAddr->setSockAddrInet(addr);
	return connFd;
}

void Socket::setReuse(bool on)
{
	int optVal = on ? 1 : 0;
	int ret = ::setsockopt(sockFd_, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));

	if(ret == -1)
	{
		LOG_SYSERR << "Socket::setReuse()";
	}
}
