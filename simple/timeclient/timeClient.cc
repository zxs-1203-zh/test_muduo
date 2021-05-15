#include "timeClient.h"



int main(int argc, char **argv)
{
	if(argc > 1)
	{
		EventLoop loop;
		InetAddress servAddr(argv[1], 2037);

		TimeClient client(&loop, servAddr);

		client.connect();

		loop.loop();
	}
	else
		printf("Usage: ./timeClient <IPaddress>\n");

	return 0;
}
