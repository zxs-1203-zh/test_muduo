#pragma once

#include <functional>

#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"

namespace muduo
{

class Acceptor//noncopyable
{

public:
	typedef std::function<void(int sockFd,
			                   const InetAddress&)>
		    NewConnectionCallback;

	Acceptor(const Acceptor&) = delete;
	Acceptor& operator=(const Acceptor&) = delete;

	Acceptor(EventLoop* loop, const InetAddress& listenAddr);

	void setNewConnectionCallback(
			const NewConnectionCallback& cb)
	{
		newConnectionCallback_ = cb;
	}

	bool listenning()const
	{
		return listenning_;
	}

	void listen();

private:
	void handleRead();

	EventLoop* loop_;
	Socket acceptSocket_;
	Channel acceptChannel_;
	bool listenning_;
	NewConnectionCallback newConnectionCallback_;

};//Acceptor

}//muduo
