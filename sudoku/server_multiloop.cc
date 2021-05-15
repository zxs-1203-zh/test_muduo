#include <muduo/base/Timestamp.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include "sudoku.h"

using namespace muduo;
using namespace muduo::net;

class SudoServer
{

public:
	SudoServer(EventLoop *loop, 
			   const InetAddress &serverAddr,
			   int numThreads)
		: server_(loop, serverAddr, "SudoServer"),
		  numThreads_(numThreads)
	{
		server_.setConnectionCallback(
				std::bind(&SudoServer::onConnection, this, _1));

		server_.setMessageCallback(
				std::bind(&SudoServer::onMessage, this, _1, _2, _3));

		server_.setThreadNum(numThreads_);
	}

	void start()
	{
		server_.start();
	}

private:

	void onConnection(const TcpConnectionPtr &conn)
	{
		LOG_INFO << "SudoServer - " << conn->peerAddress().toIpPort()
			     << " to " << conn->localAddress().toIpPort()
				 << " is " << (conn->connected() ? " UP " : " DOWN ");
	}

	void onMessage(const TcpConnectionPtr &conn,
			       Buffer *buf,
				   Timestamp time)
	{
		LOG_INFO << conn->name();
		size_t len = buf->readableBytes();
		
		while(len >= kCells + 2)
		{
			const char* crlf = buf->findCRLF();

			if(crlf)
			{
				string request(buf->peek(), crlf);
				buf->retrieveUntil(crlf + 2);
				len = buf->readableBytes();

				if(!onRequest(conn, request))
				{
					conn->send("Bad Request\r\n");
				}
			}
			else
			{
				LOG_INFO << "No CRLF";
				break;
			}
		}
	}

	bool onRequest(const TcpConnectionPtr &conn,
			     const string &request)
	{
		string id, puzzle;
		auto colon = std::find(request.begin(),
				               request.end(),
							   ':');

		if(colon != request.end())
		{
			id.assign(request.begin(), colon);
			puzzle.assign(colon + 1, request.end());
		}
		else
		{
			puzzle = request;
		}

		if(puzzle.size() == implicit_cast<size_t>(kCells))
		{
			string ans = solveSudoku(puzzle);
			
			conn->send(id + ans + "\r\n");
		}
		else
		{
			return false;
		}
			
		return true;
	}

	TcpServer server_;
	Timestamp time_;
	int numThreads_;

};//SudoServer


int main(int argc, char **argv)
{
	int numThreads = 0;
	if(argc > 1)
	{
		numThreads = atoi(argv[1]);
	}

	EventLoop loop;
	InetAddress servAddr(9981);

	SudoServer server(&loop, servAddr, numThreads);

	server.start();

	loop.loop();

	return 0;
}
