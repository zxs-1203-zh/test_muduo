#pragma once

#include <memory>
#include <string>
#include <map>

#include "TcpConnection.h"
#include "Callbacks.h"

namespace muduo
{

class Acceptor;
class InetAddress;
class EventLoop;

class TcpServer//noncopyable
{

public:
	TcpServer(const TcpServer&) = delete;
	TcpServer &operator=(const TcpServer&) = delete;

	TcpServer(EventLoop *loop, 
			  const InetAddress &listenAddr,
			  const std::string &name = std::string());

	void setMessageCallback(const MessageCallback &cb)
	{
		messageCallback_ = cb;
	}

	void setConnectionCallback(const ConnectionCallback &cb)
	{
		connectionCallback_ = cb;
	}

	void start();

private:
	typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
	void newConnection(int connFd, const InetAddress &peerAddress);

	void removeConnection(const TcpConnectionPtr &conn);

	MessageCallback messageCallback_;
	ConnectionCallback connectionCallback_;

	EventLoop *loop_;
	std::string name_;
	int connId_;
	bool started_;
	std::unique_ptr<Acceptor> acceptor_;
	ConnectionMap connections_;

};//TcpServer

}//muduo
