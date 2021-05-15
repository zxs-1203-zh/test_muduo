#include <muduo/base/Timestamp.h>
#include <muduo/base/ThreadPool.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <functional>

#include "sudoku.h"

using namespace muduo;
using namespace muduo::net;

class SudoServer
{
public:
	SudoServer(EventLoop *loop,
			   InetAddress servAddr,
			   int numThreads)
		:server_(loop, servAddr, "SudoServer"),
		 numThreads_(numThreads)
	{
		server_.setConnectionCallback(
				std::bind(&SudoServer::onConnection, this, _1));

		server_.setMessageCallback(
				std::bind(&SudoServer::onMessage, this, _1, _2, _3));
	}

	void start()
	{
		time_ = Timestamp::now();
		pool_.start(numThreads_);
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

				if(!processRequest(conn, request))
				{
					conn->send("Bad Request\r\n");
					conn->shutdown();
					break;
				}
			}
			else
			{
				break;
			}
		}
	}

	bool processRequest(const TcpConnectionPtr &conn,
		                const string &request)
	{
		string id, puzzle;
		auto colon = std::find(request.begin(), request.end(),
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
			pool_.run(std::bind(&SudoServer::solve,
						        this, conn, id, puzzle));
		}
		else
		{
			return false;
		}
		return true;
	}

	void solve(const TcpConnectionPtr& conn,
			   const string &id,
			   const string &puzzle)
	{
		string ans = solveSudoku(puzzle);

		conn->send(id + ans + "\r\n");
	}

	TcpServer server_;
	ThreadPool pool_;
	Timestamp time_;
	int numThreads_;
};//SudoServer

int main(int argc, char **argv)
{
	int numThreads = 0;
	EventLoop loop;
	if(argc > 1)
	{
		numThreads = atoi(argv[1]);
	}
	InetAddress servAddr(9981);
	SudoServer serve(&loop, servAddr, numThreads);

	serve.start();

	loop.loop();

	return 0;
}
