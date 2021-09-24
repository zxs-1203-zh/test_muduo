#pragma once

#include <arpa/inet.h>
#include <endian.h>
#include <netinet/in.h>

namespace muduo
{

namespace sockets
{

inline uint16_t hostToNetwork16(uint16_t host)
{
	return ::htons(host);
}

inline uint32_t hostToNetwork32(uint32_t host)
{
	return ::htonl(host);
}

inline uint64_t hostToNetwork64(uint64_t host)
{
	return ::htobe64(host);
}

inline uint16_t networkToHost16(uint16_t net)
{
	return ::ntohs(net);
}

inline uint32_t networkToHost32(uint32_t net)
{
	return ::ntohl(net);
}

inline uint64_t networkToHost64(uint64_t net)
{
	return ::be64toh(net);
}

int createNonblockOrDie();

void bindOrDie(int sockFd, const struct sockaddr_in& addr);

void listenOrDie(int sockFd);

int accept(int sockFd, struct sockaddr_in* addr);

void close(int sockFd);

void shutdownWrite(int sockFd);

void fromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

void toHostPort(char* buf, size_t size, const struct sockaddr_in& addr);

struct sockaddr_in getLocalAddr(int sockFd);

int getSocketError(int sockFd);

}//sockets

}//muduo
