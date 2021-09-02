#include "EventLoopThread.h"
#include <memory>
#include <mutex>
#include <thread>

#include <sys/eventfd.h>

#include <muduo/base/Logging.h>


namespace muduo
{

EventLoop* EventLoopThread::startLoop()
{
	thread_ = std::thread(std::bind(&EventLoopThread::threadFuc, this));

	std::unique_lock<std::mutex> lk(mut_);
	cond_.wait(lk, [&](){return loop_;});

	return loop_.get();
}

void EventLoopThread::threadFuc()
{
	{
		std::lock_guard<std::mutex> lk(mut_);
		loop_ = std::make_shared<EventLoop>();
		cond_.notify_all();
	}

	loop_->loop();

}

};//muduo
