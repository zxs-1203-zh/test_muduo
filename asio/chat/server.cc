#include "codec.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <set>
#include <functional>

using namespace muduo;
using namespace muduo::net;


class ChatServer: noncopyable
{

public:
	ChatServer(EventLoop *loop, const InetAddress &servAddr):
		loop_(loop),
		codec_(std::bind(&ChatServer::onStringMessage,
					     this, _1, _2, _3)),
		server_(loop, servAddr, "ChatServer")
	{
		server_.setConnectionCallback(
				std::bind(&ChatServer::onConnection,
					      this, _1));

		server_.setMessageCallback(
				std::bind(&LengthHeaderCodec::onMessage,
					      &codec_, _1, _2, _3));
	}

	void start()
	{
		server_.start();
	}


private:
	typedef std::set<TcpConnectionPtr> ConnetionList;

	void onConnection(const TcpConnectionPtr &conn)
	{
		LOG_INFO << "ChatServer - " 
			     << conn->peerAddress().toIpPort()
				 << " -> "
				 << conn->localAddress().toIpPort()
				 << (conn->connected() ? "UP" : "DOWN");

		if(conn->connected())
		{
			connections_.insert(conn);
		}
		else
		{
			connections_.erase(conn);
		}
	}

	void onStringMessage(const TcpConnectionPtr &,
		                 const string &msg,
						 Timestamp time)
	{
		for(auto &conn : connections_)
		{
			codec_.send(conn, msg);
		}
	}

	EventLoop *loop_;
	LengthHeaderCodec codec_;
	TcpServer server_;
	ConnetionList connections_;

};//ChatServer

int main(int argc, char **argv)
{
	LOG_INFO << "pid = " << ::getpid();
	if(argc > 1)
	{
		uint16_t port = static_cast<uint16_t>(atoi(argv[1]));

		EventLoop loop;
		InetAddress servAddr(port);
		ChatServer server(&loop, servAddr);

		server.start();
		loop.loop();
	}
	else
	{
		fprintf(stderr, "Usage: %s port\n", argv[0]);
	}
	return 0;
}
