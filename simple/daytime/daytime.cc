#include "daytime.h"


int main()
{
	EventLoop loop;
	InetAddress servAddr(2013);

	DaytimeServer server(&loop, servAddr);

	server.start();

	loop.loop();
}
