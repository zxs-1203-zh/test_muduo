#include "codec.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoopThread.h>

#include <iostream>
#include <mutex>

using namespace muduo;
using namespace muduo::net;

class ChatClient
{

public:
	ChatClient(EventLoop *loop, const InetAddress &servAddr):
		loop_(loop),
		codec_(std::bind(&ChatClient::onStringMessage,
					     this, _1, _2, _3)),
		client_(loop, servAddr, "ChatClient")
	{
		client_.setConnectionCallback(std::bind(&ChatClient::onConnection, 
					                            this, _1));

		client_.setMessageCallback(std::bind(&LengthHeaderCodec::onMessage,
				                             &codec_, _1, _2, _3));

		client_.enableRetry();
	}

	void connect()
	{
		client_.connect();
	}

	void disconnect()
	{
		client_.disconnect();
	}

	void write(const StringPiece& line)
	{
		std::lock_guard<std::mutex> lk(mut_);
		if(connptr_)
		{
			codec_.send(connptr_, line);
		}
	}

private:
	void onConnection(const TcpConnectionPtr &conn)
	{
		LOG_INFO << "ChatClient - " << conn->localAddress().toIpPort()
			     << " -> " << conn->peerAddress().toIpPort()
				 << " is " << (conn->connected() ? " UP " : " DOWN ");

		std::lock_guard<std::mutex> lk(mut_);

		if(conn->connected())
		{
			connptr_ = conn;
		}
		else
		{
			connptr_.reset();
		}
	}

	void onStringMessage(const TcpConnectionPtr&,
			             const string &msg,
						 Timestamp)
	{
		printf(">>> %s\n", msg.c_str());
	}

	mutable std::mutex mut_; //for connptr_
	EventLoop *loop_;
	LengthHeaderCodec codec_;
	TcpClient client_;
	TcpConnectionPtr connptr_;

};//ChatClient

int main(int argc, char **argv)
{
	if(argc > 2)
	{
		uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
		InetAddress servAddr(argv[1], port);
		EventLoopThread loopThread;
		ChatClient client(loopThread.startLoop(), servAddr);

		client.connect();

		string line;
		while(std::getline(std::cin, line))
		{
			client.write(line);
		}

		client.disconnect();
	}
	else
	{
		fprintf(stderr, "Usage %s, hostid port\n", argv[0]);
	}
}
