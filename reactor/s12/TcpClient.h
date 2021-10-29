#pragma once

#include <memory>
#include <atomic>
#include <mutex>

#include "EventLoop.h"
#include "InetAddress.h"
#include "Connector.h"
#include "Callbacks.h"

namespace muduo
{

class TcpClient//noncopyable
{

public:
	TcpClient(const TcpClient&) = delete;
	TcpClient& operator=(const TcpClient&) = delete;

	TcpClient(EventLoop *loop, const InetAddress &peerAddr);

	~TcpClient();

	void enableRetry()
	{
		retry_.store(true);
	}

	void setConnectionCallback(const ConnectionCallback &cb)
	{
		connectionCallback_ = cb;
	}

	void setMessageCallback(const MessageCallback &cb)
	{
		messageCallback_ = cb;
	}

	void setWriteCompleteCallback(const WriteCompleteCallback &cb)
	{
		writeCompleteCallback_ = cb;
	}

	void connect();

	void disconnect();

	void stop();

private:
	void newConnection(int sockFd);

	void removeConnection(const TcpConnectionPtr &conn);

	EventLoop *loop_;
	InetAddress peerAddr_;
	Connector connector_;
	std::atomic_bool retry_;
	std::atomic_bool connect_;
	int connId_;
	TcpConnectionPtr connection_; //guarded by mut_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;
	std::mutex mut_;

};//TcpClient

}//muduo
