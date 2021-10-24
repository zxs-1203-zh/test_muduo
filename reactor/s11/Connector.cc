#include "Connector.h"
#include "SocketsOps.h"
#include <algorithm>
#include <memory>

using namespace muduo;

const int Connector::kMaxRetryDelayMs;

void Connector::restart()
{
	loop_->assertInLoopThread();
	setState(kDisconnected);
	retryDelayMs_ = kInitRetryDelayMs;
	connect_ = false;
	startInLoop();
}

void Connector::startInLoop()
{
	loop_->assertInLoopThread();
	assert(state_ == kDisconnected);
	if(connect_)
	{
		connect();
	}
	else
	{
		LOG_DEBUG << "Do not connect";
	}
}

void Connector::retry(int sockFd)
{
	sockets::close(sockFd);
	setState(kDisconnected);
	if(connect_)
	{
		LOG_INFO << "Connector::retry() - Retry connecting to "
			     << servAddr_.toHostPort() << " in "
				 << retryDelayMs_ << " miliseconds";
		timerId_ = loop_->runAfter(retryDelayMs_ / 1000.0, std::bind(&Connector::startInLoop, this));
		retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
	}
	else
	{
		LOG_DEBUG << "Do not connect";
	}
}

void Connector::connect()
{
	loop_->assertInLoopThread();

	int sockFd = sockets::createNonblockOrDie();
	int ret = sockets::connect(sockFd, servAddr_.getSockAddrInet());
	int savedErrNo = 0;
	if(ret != 0)
	{
		savedErrNo = errno;
	}

	switch(savedErrNo)
	{
		case 0:
		case EINPROGRESS:
		case EINTR:
		case EISCONN:
			connecting(sockFd);
			break;

		case EAGAIN:
		case EADDRINUSE:
		case EADDRNOTAVAIL:
		case ECONNREFUSED:
		case ENETUNREACH:
			retry(sockFd);
			break;

		case EACCES:
		case EPERM:
		case EALREADY:
		case EAFNOSUPPORT:
		case EBADF:
		case EFAULT:
		case ENOTSOCK:
			LOG_SYSERR << "Connector::connect() savedErrNo = " << savedErrNo;
			sockets::close(sockFd);
			break;

		default:
			LOG_SYSERR << "Connector::connect() unkonwn error, savedErrNo = " << savedErrNo;
			sockets::close(sockFd);
			break;
	}
}

void Connector::connecting(int sockFd)
{
	loop_->assertInLoopThread();

	setState(kConnecting);
	channel_.reset(new Channel(loop_, sockFd));
	channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
	channel_->setErrorCallback(std::bind(&Connector::handleError, this));
	channel_->enableWriting();
}

void Connector::handleWrite()
{
	if(state_ == kConnecting)
	{
		int sockFd = removeAndResetChannel();
		int err = sockets::getSocketError(sockFd);
		if(err)
		{
			LOG_WARN << "Connector::handleWrite() - SO_ERROR = " << err
				      << " " << strerror(err);
		}
		else if(sockets::isSelfConnect(sockFd))
		{
			LOG_WARN << "Connector::handleWrite() - Self Connect";
			retry(sockFd);
		}
		else
		{
			setState(kConnected);
			if(connect_)
			{
				newConnectionCallback_(sockFd);
			}
			else
			{
				sockets::close(sockFd);
			}
		}
	}
	else
	{
		assert(state_ == kDisconnected);
	}
}

void Connector::handleError()
{
	LOG_ERROR << "Connector::handleError()";
	assert(state_ == kConnecting);
	int sockFd = removeAndResetChannel();
	int err = sockets::getSocketError(sockFd);
	LOG_TRACE << "SO_ERROR = " << err << " " << strerror(err);
	retry(sockFd);
}

int Connector::removeAndResetChannel()
{
	loop_->assertInLoopThread();
	channel_->disableAll();
	loop_->removeChannel(channel_.get());
	int sockFd = channel_->fd();
	loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
	return sockFd;
}
