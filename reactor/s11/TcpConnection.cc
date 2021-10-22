#include "TcpConnection.h"
#include "Socket.h"
#include "Channel.h"
#include "SocketsOps.h"
#include <functional>
#include <string>
#include <unistd.h>

using namespace muduo;
using namespace std::placeholders;

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
	channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, _1));
	channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

void TcpConnection::setTcpNoDelay(bool on)
{
	socket_->setTcpNoDelay(on);
}

void TcpConnection::send(const std::string &msg)
{
	if(connected())
	{
		if(loop_->isInLoopThread())
		{
			sendInLoop(msg);
		}
		else
		{
			loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, msg));
		}
	}
}

void TcpConnection::shutdown()
{
	if(connected())
	{
		setState(kDisConnecting);
		loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
	}
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

void TcpConnection::handleRead(Timestamp receiveTime)
{
	int savedErrno = 0;
	ssize_t n = inputBuffer_.readFd(socket_->fd(), &savedErrno);

	if(n > 0)
	{
		messageCallback_(shared_from_this(), 
				         &inputBuffer_, 
						 receiveTime);
	}
	else if(n == 0)
	{
		handleClose();
	}
	else
	{
		errno = savedErrno;
		LOG_SYSERR << "TcpConnection::handleRead()";
		handleError();
	}
}

void TcpConnection::handleWrite()
{
	loop_->assertInLoopThread();
	ssize_t nWrote = ::write(channel_->fd(),
			                  outputBuffer_.peek(),
							  outputBuffer_.readableBytes());

	if(channel_->isWriting())
	{
		if(nWrote > 0)
		{
			outputBuffer_.retrieve(nWrote);
			if(outputBuffer_.readableBytes() == 0)
			{
				channel_->disableWriting();
				if(writeCompleteCallback_)
				{
					loop_->queueInLoop(std::bind(writeCompleteCallback_, 
								                 shared_from_this()));
				}
				if(state_ == kDisConnecting)
				{
					shutdownInLoop();
				}
			}
			else
			{
				LOG_TRACE << "there are more data to write";
			}
		}
		else
		{
			LOG_SYSERR << "TcpConnection::handleRead";
		}
	}
	else
	{
		LOG_TRACE << "Connection is down, no more write";
	}
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
	int errNo = sockets::getSocketError(channel_->fd());
	LOG_ERROR << "TcpConnection::handleError() errno = "
		      << errNo << " " << strerror_tl(errNo);
}

void TcpConnection::sendInLoop(const std::string &msg)
{
	loop_->assertInLoopThread();
	ssize_t nWrote = 0;
	if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
	{
		nWrote = ::write(channel_->fd(), msg.data(), msg.size());
		if(nWrote >= 0)
		{
			if(static_cast<size_t>(nWrote) < msg.size())
			{
				LOG_TRACE << "there are more data to write";
			}
			else if(writeCompleteCallback_) //if nWrote == msg.size()
			{
				writeCompleteCallback_(shared_from_this());
			}
		}
		else
		{
			nWrote = 0;
			if(errno != EWOULDBLOCK)
			{
				LOG_ERROR << "TcpConnection::sendInLoop()";
			}
		}
	}

	assert(nWrote >= 0);
	if(static_cast<size_t>(nWrote) < msg.size())
	{
		channel_->enableWriting();
		outputBuffer_.append(msg.data() + nWrote, msg.size() - nWrote);
	}
}

void TcpConnection::shutdownInLoop()
{
	loop_->assertInLoopThread();
	if(!channel_->isWriting())
	{
		socket_->shutdownWrite();
	}
}
