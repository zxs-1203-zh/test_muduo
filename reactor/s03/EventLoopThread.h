#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include <mutex>
#include <condition_variable>
#include <thread>

#include "Channel.h"
#include "EventLoop.h"

namespace muduo
{

class EventLoopThread//noncopyable
{

public:
	EventLoopThread(const EventLoopThread&) = delete;
	EventLoopThread& operator=(const EventLoopThread&) = delete;

	EventLoopThread():
		loop_(nullptr),
		exiting_(false)
	{}

	~EventLoopThread()
	{
		exiting_ = true;
		loop_->quit();
		thread_.join();
	}

	EventLoop* startLoop();

private:
	void threadFuc();

	EventLoop *loop_; //guraded by mut_
	bool exiting_;
	std::thread thread_;
	mutable std::mutex mut_;
	std::condition_variable cond_;


};//EventLoopThread

}//muduok

#endif
