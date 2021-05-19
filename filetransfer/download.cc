#include <muduo/base/Timestamp.h>
#include <muduo/base/Logging.h>
#include <muduo/base/FileUtil.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <fstream>


using namespace muduo;
using namespace muduo::net;

class DownloadServer
{

public:
	DownloadServer(EventLoop *loop, 
			       const InetAddress &servAddr,
				   string filename)
		:loop_(loop), 
		 server_(loop, servAddr, "DownloadServer"),
		 filename_(filename)
	{
		server_.setConnectionCallback(
				std::bind(&DownloadServer::onConnetion, this, _1));
	}

	void start()
	{
		server_.start();
	}

private:
	void onConnetion(const TcpConnectionPtr &conn)
	{
		LOG_INFO << "DownloadServer - " << conn->peerAddress().toIpPort()
			     << " to " << conn->localAddress().toIpPort()
				 << " is " << (conn->connected() ? " UP " : " DOWN ");

		if(conn->connected())
		{
			LOG_INFO << "DownloadServer - sending " << filename_
				     << " to "  << conn->peerAddress().toIpPort();

			string content(readFile());

			conn->send(content);

			LOG_INFO << "DownloadServer - down";
		}
	}

	string readFile()
	{
		string content;
		FileUtil::readFile(filename_, 64 * 1024, &content);
		return content;
	}

	EventLoop *loop_;
	TcpServer server_;
	string filename_;
};//DownloadServer

int main(int argc, char** argv)
{
	EventLoop loop;
	InetAddress servAddr(2021);
	if(argc > 1)
	{
		DownloadServer server(&loop, servAddr, argv[1]);

		server.start();

		loop.loop();
	}
	else
	{
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
	}
	return 0;
}
