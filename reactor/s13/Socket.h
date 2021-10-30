#pragma once


namespace muduo
{

class InetAddress;

class Socket//noncopyable
{

public:
	Socket(const Socket&) = delete;
	Socket& operator=(const Socket&) = delete;

	Socket(int sockFd):
		sockFd_(sockFd)
	{ }

	~Socket();

	void bind(const InetAddress& addr);

	void listen();

	int accept(InetAddress* addr);

	int fd()const
	{
		return sockFd_;
	}

	void shutdownWrite();

	void setReuse(bool on);

	void setTcpNoDelay(bool on);

private:
	int sockFd_;

};//Socket

}//muduo
