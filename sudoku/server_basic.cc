#include "sudoku.h"

#include <muduo/base/Timestamp.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <functional>

using namespace muduo;
using namespace muduo::net;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;


class SudoServer
{

public:
	SudoServer(EventLoop *loop, const InetAddress &servAddr)
		: loop_(loop), server_(loop, servAddr, "SudoServer")
	{
		server_.setConnectionCallback(
				std::bind(&SudoServer::onConnection, 
					      this, _1));

		server_.setMessageCallback(
				std::bind(&SudoServer::onMessage,
					      this, _1, _2, _3));
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

				if(!processRequest(conn, request))
				{
					conn->send("Bad Request!\r\n");
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
				   const string& request)
	{
		string id, puzzle;

		auto it = std::find(request.begin(), request.end(),
				            ':');

		if(it != request.end())
		{
			id.assign(request.begin(), it);
			puzzle.assign(it + 1, request.end());
		}
		else
		{
			puzzle = request;
		}

		if(puzzle.size() == implicit_cast<size_t>(kCells))
		{
			string ansewer = solveSudoku(puzzle);

			if(!id.empty())
			{
				conn->send(id + ansewer + "\r\n");
			}
			else
			{
				conn->send(ansewer + "\r\n");
			}
		}
		else
		{
			return false;
		}

		return true;;
	}

	EventLoop *loop_;
	TcpServer server_;

};


int main()
{
	InetAddress servAddr(9981);
	EventLoop loop;
	SudoServer serve(&loop, servAddr);

	serve.start();

	loop.loop();
}
