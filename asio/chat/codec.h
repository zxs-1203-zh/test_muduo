#ifndef CONDEC_H
#define CONDEC_H

#include <functional>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpConnection.h>

using namespace muduo;
using namespace muduo::net;

class LengthHeaderCodec
{

public:
	typedef std::function<void(const TcpConnectionPtr &,
							   const string&,
							   Timestamp)> 
		StringMessageCallback;

	explicit LengthHeaderCodec(const StringMessageCallback &cb)
		:messageCallback_(cb)
	{ }

	void onMessage(const TcpConnectionPtr &conn,
				   Buffer *buf,
			       Timestamp time)
	{
		while(buf->readableBytes() >= kHeaderLen)
		{
			int32_t len = buf->peekInt32();

			if(len > 65535 || len < 0)
			{
				LOG_ERROR << "Invalid length " << len;
				conn->shutdown();
				break;
			}
			else if(buf->readableBytes() >= len + kHeaderLen)
			{
				buf->retrieve(kHeaderLen);
				string msg(buf->peek(), len);
				messageCallback_(conn, msg, time);
				buf->retrieve(msg.size());
			}
			else
			{
				break;
			}
		}
	}

	void send(const TcpConnectionPtr &conn,
			  const StringPiece& msg)
	{
		Buffer buf;
		buf.append(msg.data(), msg.size());
		int32_t len = static_cast<int32_t>(msg.size());
		int32_t be32 = sockets::hostToNetwork32(len);
		buf.prepend(&be32, sizeof(be32));
		conn->send(&buf);
	}



private:
	const static int kHeaderLen = sizeof(int32_t);
	StringMessageCallback messageCallback_;

};//LengthHeaderCodec


#endif
