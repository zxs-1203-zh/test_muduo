#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"

#include <muduo/base/Thread.h>
#include <muduo/base/Logging.h>

namespace muduo
{

__thread EventLoop* t_loopInThisThread = nullptr;
const int kMsTimeout = 10000;

EventLoop::EventLoop():
	looping_(false),
	quit_(false),
	threadId_(CurrentThread::tid()),
	poller_(new Poller(this))
{
	LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
	if(t_loopInThisThread != nullptr)
	{
		LOG_FATAL << "Another EventLoop " << t_loopInThisThread
			      << " exist in this thread " << threadId_;
	}
	else
	{
		t_loopInThisThread = this;
	}
}

EventLoop::~EventLoop()
{
	assert(!looping_);
	t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
	asserInLoopThread();
	assert(!looping_);

	looping_ = true;
	quit_ = false;

	while(!quit_)
	{
		activeChannels_.clear();
		poller_->poll(kMsTimeout, &activeChannels_);

		for(auto channel: activeChannels_)
		{
			channel->handleEvent();
		}

	}

	looping_ = false;
	LOG_TRACE << "EventLoop " << this << "stop looping";
}

void EventLoop::quit()
{
	quit_ = true;
}

void EventLoop::updateChannel(Channel *channel)
{
	asserInLoopThread();
	assert(channel->ownerLoop() == this);
	poller_->updateChannel(channel);
}

void EventLoop::asserInLoopThread()
{
	if(threadId_ != CurrentThread::tid())
	{
		abortNotInLoopThread();
	}
}

void EventLoop::abortNotInLoopThread()
{
	LOG_FATAL << "EventLoop::abortNotInLoopThread()  - EventLoop "
		      << this << " created in threadId_ " << threadId_
			  << ", currentThread is " << CurrentThread::tid();
}

}//muduo
