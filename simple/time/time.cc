#include "time.h"


int main()
{
	EventLoop loop;
	InetAddress servAddr(2037);

	TimeServer server(&loop, servAddr);

	server.start();

	loop.loop();
}
