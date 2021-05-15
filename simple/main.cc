#include "echo/EchoServer.h"
#include "time/time.h"
#include "daytime/daytime.h"
#include "discard/discard.h"


int main()
{
	LOG_INFO << "pid = " << getpid();

	EventLoop loop;

	EchoServer echoServer(&loop, InetAddress(2007));
	echoServer.start();

	TimeServer timeServer(&loop, InetAddress(2037));
	timeServer.start();

	DiscardServer discardServer(&loop, InetAddress(2009));
	discardServer.start();

	DaytimeServer daytimeServer(&loop, InetAddress(2013));
	daytimeServer.start();

	loop.loop();

	return 0;
}
