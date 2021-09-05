#include "TcpConnection.h"
#include "Socket.h"
#include "Channel.h"
#include <unistd.h>

using namespace muduo;

TcpConnection::TcpConnection(EventLoop *loop,
			  const std::string &name,
			  int connFd,
			  const InetAddress &localAddr,
			  const InetAddress &peerAddr):
	loop_(loop),
	name_(name),
	socket_(new Socket(connFd)),
	channel_(new Channel(loop, connFd)),
	localAddr_(localAddr),
	peerAddr_(peerAddr)
{
	assert(loop_ != nullptr);
	channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
}

void TcpConnection::connectEstablished()
{
	loop_->assertInLoopThread();
	assert(!connected());
	setState(kConnected);

	channel_->enableReading();
	connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead()
{
	char buf[65536];
	ssize_t n = ::read(socket_->fd(), buf, sizeof buf);
	messageCallback_(shared_from_this(), buf, n);
}
