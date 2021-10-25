#pragma once

#include <functional>
#include <memory>
#include <atomic>

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
		state_(kDisconnected),
		connect_(false),
		retryDelayMs_(kInitRetryDelayMs)
	{
		LOG_DEBUG << "Connector ctor[" << this << "]";
	}

	~Connector()
	{
		LOG_DEBUG << "Connector dtor[" << this << "]";
		loop_->cancel(timerId_);
		assert(!channel_); //why?
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

	void restart()
	{
		loop_->assertInLoopThread();
		connect_ = true;
		setState(kDisconnected);
		startInLoop();
	}

	void stop() //how this function close the sockFd?
	{
		connect_ = false;
		loop_->cancel(timerId_);
	}

private:
	enum States{kDisconnected, kConnecting, kConnected};

	void setState(States state)
	{
		state_ = state;
	}

	void startInLoop();

	void connect();

	void connecting(int sockFd);

	int removeAndResetChannel();

	void resetChannel()
	{
		channel_.reset();
	}

	void retry(int sockFd);

	void handleWrite();

	void handleError();

	EventLoop *loop_;
	InetAddress servAddr_;
	std::atomic<States> state_;
	std::atomic_bool connect_;
	int64_t retryDelayMs_;
	std::unique_ptr<Channel> channel_;
	TimerId timerId_;
	NewConnectionCallback newConnectionCallback_;

	const static int64_t kInitRetryDelayMs;
	const static int64_t kMaxRetryDelayMs;

};//Connector

typedef std::shared_ptr<Connector> ConnectorPtr;

}//muduo
