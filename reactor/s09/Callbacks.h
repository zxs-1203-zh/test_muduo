#pragma once

#include <functional>
#include <memory>


namespace muduo
{

class TcpConnection;
class InetAddress;
class Timestamp;
class Buffer;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void()> TimerCallback;
typedef std::function<void()> EventCallback;
typedef std::function<void(Timestamp)> ReadEventCallback;

typedef std::function<void(int connFd, const InetAddress& peerAddr)> NewConnectionCallback;

typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr&,
		                   Buffer *,
						   Timestamp)> MessageCallback;
};//muduo
