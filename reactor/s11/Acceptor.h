#pragma once

#include <functional>

#include "Socket.h"
#include "Channel.h"
#include "InetAddress.h"
#include "EventLoop.h"

namespace muduo
{

typedef std::function<void(int, const InetAddress&)> NewConnectionCallback;

class Acceptor//noncopyable
{

public:
	Acceptor(const Acceptor&) = delete;
	Acceptor& operator=(const Acceptor&) = delete;

	Acceptor(EventLoop* loop, const InetAddress& localAddr);

	void listen();

	bool listenning()const
	{
		return listenning_;
	}

	void setNewConnectionCallback(const NewConnectionCallback& cb)
	{
		newConnectionCallback_ = cb;
	}

private:
	void handleRead();

	EventLoop* 			  loop_;
	Socket     			  acceptSocket_;
	Channel    			  acceptChannel_;
	bool 				  listenning_;
	NewConnectionCallback newConnectionCallback_;

};//Acceptor

}//muduo
