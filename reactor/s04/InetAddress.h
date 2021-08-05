#pragma once

#include <netinet/in.h>
#include <string>

namespace muduo
{

class InetAddress//copyable
{

public:
	InetAddress() = default;
	~InetAddress() = default;
	//default copy/assignment are Ok
	
	InetAddress(uint16_t port);

	InetAddress(const struct sockaddr_in& addr):
		addr_(addr)
	{ }

	InetAddress(const std::string& ip, uint16_t port);

	std::string toHostPort()const;

	const struct sockaddr_in& getSockAddrInet()const
	{
		return addr_;
	}

	void setSockAddrInet(const struct sockaddr_in& addr)
	{
		addr_ = addr;
	}

private:
	struct sockaddr_in addr_;

};//InetAddress

}//muduo
