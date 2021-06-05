#include "EventLoopThread.h"
#include <mutex>

namespace muduo
{

EventLoopThread::EventLoopThread():
	thread_(std::bind(&EventLoopThread::threadFuc, this)),
	loop_(nullptr),
	exiting_(false)
{ }

EventLoopThread::~EventLoopThread()
{
	exiting_ = true;
	loop_->quit();
	thread_.join();
}

EventLoop* EventLoopThread::startLoop()
{
	assert(!thread_.started());
	thread_.start();

	std::unique_lock<std::mutex> lk(mut_);
	cond_.wait(lk, [=](){return loop_ != nullptr;});

	return loop_;
}

void EventLoopThread::threadFuc()
{
	EventLoop loop;

	{
		std::lock_guard<std::mutex> lk(mut_);
		loop_ = &loop;
	}

	loop_->loop();
}

}//muduo
