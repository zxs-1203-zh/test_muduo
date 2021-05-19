#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>

#include <fstream>

using namespace muduo;
using namespace muduo::net;

class DownloadClient
{

public:
	DownloadClient(EventLoop *loop, InetAddress servAddr)
		:loop_(loop),
		 client_(loop, servAddr, "DownloadClient"),
		 fout_("download.txt")
	{ 
		client_.setConnectionCallback(
				std::bind(&DownloadClient::onConnection, this, _1));

		client_.setMessageCallback(
				std::bind(&DownloadClient::onMessage, this, _1, _2, _3));
	}

	void connet()
	{
		client_.connect();
	}

private:
	void onConnection(const TcpConnectionPtr &conn)
	{
		LOG_INFO << "TcpClient - " << conn->localAddress().toIpPort()
			     << " to " << conn->peerAddress().toIpPort()
				 << " is " << (conn->connected() ? " UP " : " DOWN ");

		if(!conn->connected())
		{
			conn->shutdown();
			loop_->quit();
		}
	}

	void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
	{

		string msg(buf->retrieveAllAsString());

		fout_ << msg;
	}

	EventLoop *loop_;
	TcpClient client_;
	std::ofstream fout_;

};//DownloadClient


int main()
{
	EventLoop loop;
	InetAddress servAddr(2021);
	DownloadClient client(&loop, servAddr);

	client.connet();

	loop.loop();

	return 0;
}
