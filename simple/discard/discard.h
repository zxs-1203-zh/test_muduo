#ifndef DISCARD_H
#define DISCARD_H

#include <muduo/base/Timestamp.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

using namespace muduo;
using namespace muduo::net;

class DiscardServer
{

public:
	DiscardServer(EventLoop *loop,
			      const InetAddress& servAddr)
		:server_(loop, servAddr, "DiscardServer")
	{
		server_.setConnectionCallback(
				std::bind(&DiscardServer::onConnection,
					      this, _1));

		server_.setMessageCallback(
				std::bind(&DiscardServer::onMessage,
					      this, _1, _2, _3));
	}

	void start()
	{
		server_.start();
	}

private:
	void onConnection(const TcpConnectionPtr &conn)
	{
		LOG_INFO << "DiscardServer - "
			     << conn->peerAddress().toIpPort()
				 << " to " << conn->localAddress().toIpPort()
				 << " is "
				 << (conn->connected() ? " UP " : " DOWN ");
	}

	void onMessage(const TcpConnectionPtr &conn,
			       Buffer *buf,
				   Timestamp time)
	{
		string msg(buf->retrieveAllAsString());

		LOG_INFO << conn->name() << " discards "
			     << msg.size()   << " bytes at "
				 << Timestamp::now().toFormattedString();
	}

	TcpServer server_;

};//DiscardServer


#endif
