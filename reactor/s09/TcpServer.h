#pragma once

#include <memory>
#include <map>
#include <string>

#include "Callbacks.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "TcpConnection.h"

namespace muduo
{

class TcpServer//noncopyable
{

public:
	TcpServer(const TcpServer&) = delete;
	TcpServer& operator=(const TcpServer&) = delete;

	TcpServer(EventLoop *loop, const InetAddress &listenAddr);

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

	void start();

private:
	void newConnection(int connFd, const InetAddress &peerAddr);

	void removeConnection(const TcpConnectionPtr& conn);

	typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;
	EventLoop *loop_;
	std::string name_;
	std::unique_ptr<Acceptor> acceptor_;
	int nextConnId_;
	bool started_;
	ConnectionMap connectionMap_;

};//TcpServer

}//muduo
