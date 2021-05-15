#ifndef TIMECLIENT_H
#define TIMECLIENT_H

#include <muduo/base/Timestamp.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>

using namespace muduo;
using namespace muduo::net;

class TimeClient
{
public:
	TimeClient(const TimeClient&) = delete;
	TimeClient& operator=(const TimeClient&) = delete;

	TimeClient(EventLoop *loop,
			   InetAddress servAddr)
		: loop_(loop), client_(loop, servAddr, "TimeClient")
	{
		client_.setConnectionCallback(
				std::bind(&TimeClient::onConnetion, this, _1));

		client_.setMessageCallback(
				std::bind(&TimeClient::onMessage, this, _1,
					      _2, _3));
	}
	
	void connect()
	{
		client_.connect();
	}

private:
	void onConnetion(const TcpConnectionPtr &conn)
	{
		LOG_INFO << "TimeClient - "
			     << conn->localAddress().toIpPort()
				 << " to "
				 << conn->peerAddress().toIpPort()
				 << " is "
				 << (conn->connected() ? " UP " : "DOWN");

		if(!conn->connected())
		{
			loop_->quit();
		}
	}

	void onMessage(const TcpConnectionPtr &conn,
			       Buffer *buf,
				   Timestamp time)
	{
		const void *data = buf->peek();

		int32_t be32 = *static_cast<const int32_t*>(data);

		buf->retrieve(sizeof(int32_t));

		time_t tm_t = static_cast<time_t>(sockets::networkToHost32(be32));

		Timestamp tm(tm_t * Timestamp::kMicroSecondsPerSecond);

		LOG_INFO << tm.toFormattedString();
	}

	TcpClient client_;
	EventLoop *loop_;
};//TimeClient


#endif
