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
		:client_(loop, servAddr, "DownloadClient")
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
		LOG_INFO << "TcpClient - " << conn->localAddress().toPort()
			     << " to " << conn->peerAddress().toPort()
				 << " is " << (conn->connected() ? " UP " : " DOWN ");
	}

	void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
	{
		std::ofstream fout("dowload.txt");

		string msg(buf->retrieveAllAsString());

		LOG_INFO << "Downloading download.txt";

		fout << msg;

		LOG_INFO << "Done";
	}

	TcpClient client_;

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
