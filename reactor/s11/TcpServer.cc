#include <cstdio>
#include <functional>

#include "TcpServer.h"
#include "SocketsOps.h"

using namespace muduo;
using namespace std::placeholders;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr):
	loop_(loop),
	acceptor_(new Acceptor(loop, listenAddr)),
	threadPool_(new EventLoopThreadPool(loop)),
	nextConnId_(1),
	started_(false)
{
	assert(loop_ != nullptr);
	acceptor_->setNewConnectionCallback(
			std::bind(&TcpServer::newConnection, this, _1, _2));
}

void TcpServer::setThreadNum(int numThreads)
{
	assert(numThreads >= 0);
	threadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
	if(!started_)
	{
		started_ = true;
		threadPool_->start();
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
	std::string name = name_ + buf;

	++nextConnId_;

	InetAddress localAddr(sockets::getLocalAddr(connFd));

	EventLoop *ioLoop = threadPool_->getNextLoop();

	TcpConnectionPtr conn(new TcpConnection(ioLoop, name, connFd, localAddr, peerAddr));
	connectionMap_[name] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
	ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
	loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
	loop_->assertInLoopThread();
	auto n = connectionMap_.erase(conn->name());
	assert(n == 1);
	(void)n;
	EventLoop *ioLoop = conn->getLoop();
	ioLoop->queueInLoop(std::bind(&TcpConnection::connectDistroyed, conn));
}
