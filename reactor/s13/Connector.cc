#include "Connector.h"
#include "SocketsOps.h"
#include <utility>


using namespace muduo;

const int64_t Connector::kInitRetryDelayMs = 500;
const int64_t Connector::kMaxRetryDelayMs = 30 * 1000;

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

void Connector::connect()
{
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
		case EADDRINUSE:
		case EADDRNOTAVAIL:
		case EAGAIN:
		case ECONNREFUSED:
		case ENETUNREACH:
			retry(sockFd);
			break;
		case EACCES:
		case EPERM:
		case EAFNOSUPPORT:
		case EALREADY:
		case EBADF:
		case EFAULT:
		case ENOTSOCK:
			LOG_ERROR << "Connector::connect() errno = " << savedErrNo;
			sockets::close(sockFd);
			break;
		default:
			LOG_ERROR << "Connector::connect() unknown error errno = " << savedErrNo;
			sockets::close(sockFd);
			break;
	}
}

void Connector::connecting(int sockFd)
{
	setState(kConnecting);
	channel_.reset(new Channel(loop_, sockFd));
	channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
	channel_->setErrorCallback(std::bind(&Connector::handleError, this));
	channel_->enableWriting();
}

int Connector::removeAndResetChannel()
{
	int sockFd = channel_->fd();
	channel_->disableAll();
	loop_->removeChannel(channel_.get());
	loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
	return sockFd;
}

void Connector::retry(int sockFd)
{
	sockets::close(sockFd);
	setState(kDisconnected);
	if(connect_)
	{
		LOG_INFO << "Connector::retry() - Retry connecting to "
			     << servAddr_.toHostPort() <<  " in "
				 << retryDelayMs_ << " miliseconds";
		timerId_ = loop_->runAfter(retryDelayMs_ / 1000.0, std::bind(&Connector::startInLoop, this));
		retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
	}
	else
	{
		LOG_DEBUG << "Do not connect";
	}
}

void Connector::handleWrite()
{
	LOG_TRACE << "Connector::handleWrite()";

	loop_->assertInLoopThread();
	if(state_ == kConnecting)
	{
		int sockFd = removeAndResetChannel();
		int err = sockets::getSocketError(sockFd);
		if(err)
		{
			LOG_WARN << "Connector::handleWrite() - SO_ERRNO = " << err
				     << " " << strerror_tl(err);
			retry(sockFd);
		}
		else if(sockets::isSelfConnect(sockFd))
		{
			LOG_WARN << "Connector::handleWrite() - Self Connection";
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
	LOG_TRACE << "SOERRNO = " << strerror_tl(err);
	retry(sockFd);
}
