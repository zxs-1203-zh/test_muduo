#include "SocketsOps.h"
#include "InetAddress.h"
#include "Socket.h"
#include <asm-generic/socket.h>
#include <strings.h>
#include <sys/socket.h>

#include <muduo/base/Logging.h>

using namespace muduo;

void Socket::bindAddress(const InetAddress& localAddress)
{
	sockets::bindOrDie(sockFd_, localAddress.getSockAddrInet());
}

void Socket::listen()
{
	sockets::listenOrDie(sockFd_);
}

int Socket::accept(InetAddress* peerAddress)
{
	struct sockaddr_in addr;
	::bzero(&addr, sizeof(addr));
	int connFd = sockets::accept(sockFd_, &addr);
	if(connFd > 0)
	{
		peerAddress->setSockAddrInet(addr);
	}
	return connFd;
}

void Socket::setReuseAddr(bool on)
{
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(sockFd_, SOL_SOCKET, SO_REUSEADDR,
			               &optval, sizeof(optval));
	if(ret == -1)
	{
		LOG_SYSERR << "Socket::setReuseAddr()";
	}
}
