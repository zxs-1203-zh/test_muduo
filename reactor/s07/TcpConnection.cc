#include "TcpConnection.h"
#include "Socket.h"
#include "Channel.h"
#include "SocketsOps.h"
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
	channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

void TcpConnection::connectEstablished()
{
	loop_->assertInLoopThread();
	assert(!connected());

	setState(kConnected);

	channel_->enableReading();
	connectionCallback_(shared_from_this());
}

void TcpConnection::connectDistroyed()
{
	loop_->assertInLoopThread();
	assert(connected());
	setState(kDisConnected);
	channel_->disableAll();
	connectionCallback_(shared_from_this());
	loop_->removeChannel(channel_.get());
}

void TcpConnection::handleRead()
{
	char buf[65536]; //FIXME
	ssize_t n = ::read(socket_->fd(), buf, sizeof buf);

	if(n > 0)
	{
		messageCallback_(shared_from_this(), buf, n);
	}
	else if(n == 0)
	{
		handleClose();
	}
	else
	{
		handleError();
	}
}

void TcpConnection::handleWrite()
{
	//FIXME
}

void TcpConnection::handleClose()
{
	loop_->assertInLoopThread();
	assert(connected());
	channel_->disableAll();
	closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
	int errNo = sockets::getSocketError(socket_->fd());
	LOG_ERROR << "TcpConnection::handleError() errno = "
		      << errNo << " " << strerror_tl(errNo);
}

