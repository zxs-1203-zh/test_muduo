#pragma once

namespace muduo
{

class InetAddress;

class Socket//noncopyable
{

public:
	Socket(const Socket&) = delete;
	Socket& operator=(const Socket&) = delete;

	explicit Socket(int sockFd): 
		sockFd_(sockFd)
	{}

	int fd()const
	{
		return sockFd_;
	}

	void bindAddress(const InetAddress& localAddress);

	void listen();

	int accept(InetAddress* peerAddress);

	void setReuseAddr(bool on);


private:
	int sockFd_;

};//Socket

}//muduo
