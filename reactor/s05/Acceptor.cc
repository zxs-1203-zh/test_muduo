#include "SocketsOps.h"
#include "Acceptor.h"

using namespace muduo;

Acceptor::Acceptor(EventLoop *loop, const InetAddress& localAddr):
	loop_(loop),
	acceptSocket_(sockets::createNonblockOrDie()),
	acceptChannel_(loop, acceptSocket_.fd()),
	listenning_(false)
{
	acceptSocket_.setReuse(true);
	acceptSocket_.bind(localAddr);
	acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
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

	InetAddress addr;
	int connFd = acceptSocket_.accept(&addr);
	if(connFd > 0)
	{
		if(newConnectionCallback_)
		{
			newConnectionCallback_(connFd, addr);
		}
	}
}
