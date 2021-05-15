#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <map>

using namespace muduo;
using namespace muduo::net;

std::map<string, string> users;

string getUser(const string& user)
{
	auto it = users.find(user);

	if(it != users.end())
	{
		return it->second;
	}

	return string("No such user");
}

void onConnection(const TcpConnectionPtr &conn)
{
	LOG_INFO << "Figger - " 
		     << conn->peerAddress().toIpPort()
			 << " to "
			 << conn->localAddress().toIpPort()
			 << " is "
			 << (conn->connected() ? " UP " : " DOWN ");
}

void onMessage(const TcpConnectionPtr &conn,
			   Buffer *buf,
			   Timestamp time)
{
	const char* crlf = buf->findCRLF();

	if(crlf)
	{
		string user(buf->peek(), crlf);
		string msg(getUser(user));
		conn->send(msg + "\r\n");
		buf->retrieveUntil(crlf + 2);
		conn->shutdown();
	}
}

int main()
{
	EventLoop loop;
	InetAddress listenAddr(1097);
	TcpServer server(&loop, listenAddr, "Figger");

	server.setConnectionCallback(onConnection);
	server.setMessageCallback(onMessage);

	server.start();

	loop.loop();

	return 0;
}
