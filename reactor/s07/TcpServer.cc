#include <cstdio>
#include <functional>

#include "TcpServer.h"
#include "SocketsOps.h"

using namespace muduo;
using namespace std::placeholders;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr):
	loop_(loop),
	acceptor_(new Acceptor(loop, listenAddr)),
	nextConnId_(1),
	started_(false)
{
	assert(loop_ != nullptr);
	acceptor_->setNewConnectionCallback(
			std::bind(&TcpServer::newConnection, this, _1, _2));
}

void TcpServer::start()
{
	if(!started_)
	{
		started_ = true;
	}
	if(!acceptor_->listenning())
	{
		loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
	}
}

void TcpServer::newConnection(int connFd, const InetAddress &peerAddr)
{
	loop_->assertInLoopThread();
	char buf[32];
	::snprintf(buf, sizeof buf, "%d", nextConnId_);
	++nextConnId_;
	std::string name = name_ + buf;
	InetAddress localAddr(sockets::getLocalAddr(connFd));
	TcpConnectionPtr conn(new TcpConnection(loop_, name, connFd, localAddr, peerAddr));
	connectionMap_[name] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
	conn->connectEstablished();
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
	loop_->assertInLoopThread();
	auto n = connectionMap_.erase(conn->name());
	assert(n == 1);
	(void)n;
	loop_->queueInLoop(std::bind(&TcpConnection::connectDistroyed, conn));
}
