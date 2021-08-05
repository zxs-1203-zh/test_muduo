#include "Acceptor.h"
#include "SocketsOps.h"

using namespace muduo;

Acceptor::Acceptor(EventLoop* loop, 
		           const InetAddress& listenAddr):
	loop_(loop),
	acceptSocket_(sockets::createNoblockingOrDie()),
	acceptChannel_(loop, acceptSocket_.fd()),
	listenning_(false)
{
	acceptSocket_.setReuseAddr(true);
	acceptSocket_.bindAddress(listenAddr);
	acceptChannel_.setReadCallback(
			std::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen()
{
	loop_->assertInLoopThread();
	listenning_ = true;
	acceptSocket_.listen();
	acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
	loop_->assertInLoopThread();
	InetAddress peerAddr;
	int connFd = acceptSocket_.accept(&peerAddr);

	if(connFd > 0)
	{
		if(newConnectionCallback_)
		{
			newConnectionCallback_(connFd, peerAddr);
		}
	}
}
