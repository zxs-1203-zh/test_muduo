#include "InetAddress.h"
#include "SocketsOps.h"

#include <cstring>

using namespace muduo;

InetAddress::InetAddress(uint16_t port)
{
	::bzero(&addr_, sizeof(addr_));
	addr_.sin_family = AF_INET;
	addr_.sin_addr.s_addr = INADDR_ANY;
	addr_.sin_port = sockets::hostToNetwork16(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
	::bzero(&addr_, sizeof(addr_));
	sockets::fromHostPort(ip.c_str(), port, &addr_);
}

std::string InetAddress::toHostPort()const
{
	char buf[32];
	sockets::toHostPort(buf, sizeof(buf), addr_);
	return buf;
}
