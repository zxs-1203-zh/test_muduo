#pragma once

#include <memory>
#include <string>

#include <muduo/base/Timestamp.h>

#include "Callbacks.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Buffer.h"
#include "Socket.h"

namespace muduo
{

class Channel;

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

	void setCloseCallback(const CloseCallback &cb)
	{
		closeCallback_ = cb;
	}

	void setWriteCompleteCallback(const WriteCompleteCallback &cb)
	{
		writeCompleteCallback_ = cb;
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

	void setTcpNoDelay(bool on);

	void send(const std::string &msg);

	void shutdown();

	void connectEstablished();

	void connectDistroyed();

private:
	void handleRead(Timestamp receiveTime);

	void handleWrite();

	void handleClose();

	void handleError();

	void sendInLoop(const std::string &msg);

	void shutdownInLoop();

	enum StateE {kConncting, kConnected, kDisConnecting, kDisConnected};

	void setState(StateE state)
	{
		state_ = state;
	}

	StateE state_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	CloseCallback closeCallback_;
	WriteCompleteCallback writeCompleteCallback_;
	EventLoop *loop_;
	std::string name_;
	std::unique_ptr<Socket> socket_;
	std::unique_ptr<Channel> channel_;
	InetAddress localAddr_;
	InetAddress peerAddr_;
	Buffer inputBuffer_;
	Buffer outputBuffer_;

};//TcpConnection

}//muduo
