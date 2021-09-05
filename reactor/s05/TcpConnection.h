#pragma once

#include <memory>
#include <string>

#include "Callbacks.h"
#include "InetAddress.h"
#include "EventLoop.h"

namespace muduo
{

class Channel;
class Socket;

class TcpConnection: public std::enable_shared_from_this<TcpConnection>//noncopyable
{

public:
	TcpConnection(const TcpConnection&) = delete;
	TcpConnection& operator=(const TcpConnection&) = delete;

	TcpConnection(EventLoop *loop, 
			      const std::string &name,
				  int connFd,
				  const InetAddress &localAddr,
				  const InetAddress &peerAddr);

	void setConnectionCallback(const ConnectionCallback &cb)
	{
		connectionCallback_ = cb;
	}

	void setMessageCallback(const MessageCallback &cb)
	{
		messageCallback_ = cb;
	}

	EventLoop *getLoop()const
	{
		return loop_;
	}

	std::string name()const
	{
		return name_;
	}

	const InetAddress& localAddress()const
	{
		return localAddr_;
	}

	const InetAddress& peerAddress()const
	{
		return peerAddr_;
	}

	bool connected()const
	{
		return state_ == kConnected;
	}

	void connectEstablished();

private:
	void handleRead();

	enum StateE {kConncting, kConnected};

	void setState(StateE state)
	{
		state_ = state;
	}

	StateE state_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	EventLoop *loop_;
	std::string name_;
	std::unique_ptr<Socket> socket_;
	std::unique_ptr<Channel> channel_;
	InetAddress localAddr_;
	InetAddress peerAddr_;

};//TcpConnection

}//muduo
