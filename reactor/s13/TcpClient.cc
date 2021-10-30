#include <memory>
#include <mutex>
#include <sstream>

#include "TcpClient.h"
#include "TcpConnection.h"
#include "SocketsOps.h"

using namespace muduo;
using namespace std::placeholders;

namespace muduo
{

namespace detail
{

void removeConnection(EventLoop *loop, const TcpConnectionPtr &conn)
{
}

void removeConnector()
{
	//FIXME
}

}//detail

}//muduo

TcpClient::TcpClient(EventLoop *loop, const InetAddress &peerAddr):
	loop_(loop),
	peerAddr_(peerAddr),
	connector_(loop, peerAddr),
	retry_(false),
	connect_(false),
	connId_(1)
{
	connector_.setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, _1));
}

TcpClient::~TcpClient()
{
}

void TcpClient::connect()
{
	LOG_INFO << "TcpClient::connect[" << this << "] - connecting to "
		     << peerAddr_.toHostPort();
	connect_.store(true);
	connector_.start();
}

void TcpClient::disconnect()
{
	connect_ = false;
	{
		std::lock_guard<std::mutex> lk(mut_);
		if(connection_)
		{
			connection_->shutdown();
		}
	}
} 

void TcpClient::stop()
{
	connect_.store(false);
	connector_.stop();
}

void TcpClient::newConnection(int sockFd)
{
	std::string name;
	std::stringstream nameStream;
	nameStream << peerAddr_.toHostPort() << "#" << connId_++;
	nameStream >> name;
	auto localAddr = sockets::getLocalAddr(sockFd);
	TcpConnectionPtr conn(new TcpConnection(loop_, name, sockFd, localAddr, peerAddr_));
	{
		std::lock_guard<std::mutex> lk(mut_);
		connection_ = conn;
	}

	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, _1));
	conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn)
{
	loop_->assertInLoopThread();
	assert(loop_ == conn->getLoop());
	{
		std::lock_guard<std::mutex> lk(mut_);
		assert(conn == connection_);
		connection_.reset();
	}

	loop_->queueInLoop(std::bind(&TcpConnection::connectDistroyed, conn));

	if(retry_ && connect_)
	{
		LOG_INFO << "TcpClient::removeConnection [" << this << "]"
			     << " - Reconnecting to " << peerAddr_.toHostPort();
		connector_.restart();
	}
}
