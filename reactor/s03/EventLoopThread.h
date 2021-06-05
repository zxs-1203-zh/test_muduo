#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include <mutex>
#include <condition_variable>

#include <muduo/base/Thread.h>

#include "EventLoop.h"

namespace muduo
{

class EventLoopThread//noncopyable
{

public:
	EventLoopThread(const EventLoopThread&) = delete;
	EventLoopThread& operator=(const EventLoopThread&) = delete;

	EventLoopThread();
	~EventLoopThread();

	EventLoop* startLoop();

private:
	void threadFuc();

	mutable std::mutex mut_;
	std::condition_variable cond_;
	Thread thread_;
	EventLoop *loop_;
	bool exiting_;

};//EventLoopThread

}//muduo

#endif
