#include "TcpServer.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include "TcpConnection.h"

#include <cstdio>
#include <functional>
#include <string>

using namespace muduo;
using namespace std::placeholders;

TcpServer::TcpServer(EventLoop *loop, 
		             const InetAddress &listenAddr,
					 const std::string &name):
	loop_(loop),
	name_(name),
	connId_(1),
	started_(false),
	acceptor_(new Acceptor(loop, listenAddr))
{
	assert(loop_ != nullptr);
	acceptor_->setNewConnectionCallback(
			std::bind(&TcpServer::newConnection, this, _1, _2));
}

void TcpServer::start()
{
	started_ = true;
	if(!acceptor_->listenning())
	{
		loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
	}
}

void TcpServer::newConnection(int connFd, const InetAddress &peerAddress)
{
	loop_->assertInLoopThread();
	char buf[32];
	::snprintf(buf, sizeof buf, "%d", connId_);
	++connId_;
	std::string name = name_ + buf;
	InetAddress localAddr(sockets::getLocalAddr(connFd));
	TcpConnectionPtr conn(new TcpConnection(loop_, name, connFd, localAddr, peerAddress));
	connections_[name] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
	conn->connectEstablished();
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
	loop_->assertInLoopThread();
	size_t n = connections_.erase(conn->name());
	assert(n == 1);
	(void)n;

	loop_->queueInLoop(std::bind(&TcpConnection::connectDistroyed, conn));
}
