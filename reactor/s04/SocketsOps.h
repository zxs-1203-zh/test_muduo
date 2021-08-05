#pragma once

#include <arpa/inet.h>
#include <endian.h>
#include <netinet/in.h>

namespace muduo
{

namespace sockets
{

inline uint16_t hostToNetwork16(uint16_t host16)
{
	return ::htons(host16);
}

inline uint32_t hostToNetwork32(uint32_t host32)
{
	return ::htonl(host32);
}

inline uint64_t hostToNetwork64(uint64_t host64)
{
	return ::htobe64(host64);
}

inline uint16_t networkToHost16(uint16_t net16)
{
	return ::ntohs(net16);
}

inline uint32_t networkToHost32(uint32_t net32)
{
	return ::ntohl(net32);
}

inline uint64_t networkToHost64(uint64_t net64)
{
	return ::be64toh(net64);
}

int createNoblockingOrDie();

void bindOrDie(int sockFd, 
		       const struct sockaddr_in& addr);

void listenOrDie(int sockFd);

int accept(int sockFd, struct sockaddr_in* addr);

void close(int sockFd);

void toHostPort(char* buf, 
		        size_t size,
		        const sockaddr_in& addr);

void fromHostPort(const char* ip,
		          uint16_t port,
				  struct sockaddr_in* addr);

}//sockets

}//muduok
