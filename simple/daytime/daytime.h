#ifndef DAYTIME_H
#define DAYTIME_H

#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

using namespace muduo;
using namespace muduo::net;

class DaytimeServer
{

public:
	DaytimeServer(EventLoop *loop,
			      const InetAddress &servAddr)
		:server_(loop, servAddr, "DaytimeServer")
	{
		server_.setConnectionCallback(
				std::bind(&DaytimeServer::onConnection,
					      this, _1));

		server_.setMessageCallback(
				std::bind(&DaytimeServer::onMessage,
					      this, _1, _2, _3));
	}

	void start()
	{
		server_.start();
	}

private:
	void onConnection(const TcpConnectionPtr &conn)
	{
		LOG_INFO << "DaytimeServer - "
			     << conn->peerAddress().toIpPort()
				 << " to " << conn->localAddress().toIpPort()
				 << " is "
				 << (conn->connected() ? " UP " : " DOWN");

		if(conn->connected())
		{
			conn->send(Timestamp::now().toFormattedString()
					   + "\n");

		conn->shutdown();
		}
	}

	void onMessage(const TcpConnectionPtr &conn,
			       Buffer *buf,
				   Timestamp time)
	{
		string msg(buf->retrieveAllAsString());

		LOG_INFO << conn->name() << " discards " 
			     << msg.size()   << " bytes received at "
				 << Timestamp::now().toString();
	}

	TcpServer server_;

};//DaytimeServer


#endif
