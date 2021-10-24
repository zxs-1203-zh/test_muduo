#pragma once

#include <memory>
#include <atomic>
#include <functional>

#include "Channel.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "TimerId.h"

namespace muduo
{

class Connector//noncopyalbe
{

public:
	typedef std::function<void(int sockFd)> NewConnectionCallback;

	Connector(const Connector&) = delete;
	Connector& operator=(const Connector&) = delete;

	Connector(EventLoop *loop, const InetAddress &servAddr):
		loop_(loop),
		servAddr_(servAddr),
		connect_(false),
		state_(kDisconnected),
		retryDelayMs_(kInitRetryDelayMs)
	{
		LOG_DEBUG << "ctor [" << this << "]";
	}

	~Connector()
	{
		LOG_DEBUG << "dtor [" << this << "]";
		loop_->cancel(timerId_);
		assert(!channel_);
	}

	void setNewConnectionCallback(const NewConnectionCallback &cb)
	{
		newConnectionCallback_ = cb;
	}

	void start()
	{
		connect_ = true;
		loop_->runInLoop(std::bind(&Connector::startInLoop, this));
	}

	void stop()
	{
		connect_ = false;
		loop_->cancel(timerId_);
	}

	void restart();

private:
	enum States{kConnected, kConnecting, kDisconnected};
	static const int kInitRetryDelayMs = 500;
	static const int kMaxRetryDelayMs = 30 * 1000;

	void setState(States state)
	{
		state_ = state;
	}

	void handleWrite();
	void handleError();

	void startInLoop();
	void retry(int sockFd);
	void connect();
	void connecting(int sockFd);
	int removeAndResetChannel();
	void resetChannel()
	{
		channel_.reset();
	}

	EventLoop *loop_;
	InetAddress servAddr_;
	std::atomic_bool connect_;
	std::atomic<States> state_;
	int retryDelayMs_;
	TimerId timerId_;
	std::unique_ptr<Channel> channel_;
	NewConnectionCallback newConnectionCallback_;

};//Connector

typedef std::shared_ptr<Connector> ConnectorPtr;

}//muduo
