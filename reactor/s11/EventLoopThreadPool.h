#pragma once

#include <vector>
#include <thread>
#include <memory>
#include "EventLoop.h"
#include "EventLoopThread.h"

namespace muduo
{

class EventLoopThreadPool//noncopyable
{

public:
	EventLoopThreadPool(const EventLoopThreadPool&) = delete;
	EventLoopThreadPool& operator=(const EventLoopThreadPool&) = delete;

	EventLoopThreadPool(EventLoop *baseLoop);
	~EventLoopThreadPool();

	void setThreadNum(int numThreads)
	{
		numThreads_ = numThreads;
	}

	void start();

	EventLoop *getNextLoop();

private:
	EventLoop *baseLoop_;
	bool started_;
	int next_;
	int numThreads_;
	std::vector<std::unique_ptr<EventLoopThread>> threads_;
	std::vector<EventLoop*> loops_;

};//EventLoopThreadPool

}//muduo
