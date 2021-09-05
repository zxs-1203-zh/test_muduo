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

	//default copy/assigment are Ok
	
	InetAddress(const struct sockaddr_in& addr):
		addr_(addr)
	{ }

	explicit InetAddress(uint16_t port);

	InetAddress(const std::string& ip, uint16_t port);

	std::string toHostPort()const;

	const sockaddr_in& getSockAddrInet()const
	{
		return addr_;
	}

	void setSockAddrInet(const sockaddr_in& addr)
	{
		addr_ = addr;
	}

private:
	struct sockaddr_in addr_;

};//InetAddress

}//muduo
