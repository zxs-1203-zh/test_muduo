#include "discard.h"


int main()
{
	EventLoop loop;
	InetAddress servAddr(2009);

	DiscardServer server(&loop, servAddr);

	server.start();

	loop.loop();

	return 0;
}
