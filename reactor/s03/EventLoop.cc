#include <muduo/base/Logging.h>

#include "TimerQueue.h"
#include "Channel.h"
#include "Poller.h"
#include "EventLoop.h"

namespace muduo
{

__thread EventLoop *owerLoop;

const int kTimeOutMs = 10000;

EventLoop::EventLoop():
	pthreadId_(CurrentThread::tid()),
	looping_(false),
	quit_(false),
	poller_(new Poller(this)),
	timerQueue_(new TimerQueue(this))
{
	if(owerLoop == nullptr)
	{
		owerLoop = this;
		LOG_TRACE << "EventLoop " << this 
			      << " created in thread " << pthreadId_;
	}
	else
	{
		LOG_ERROR << " this EventLoop " << this
			      << " another EventLoop " << owerLoop 
				  << " exists in this thread " << pthreadId_;
	}
}

EventLoop::~EventLoop()
{
	assert(!looping_);
	owerLoop = nullptr;
}

void EventLoop::updateChannel(Channel *channel)
{
	assertInLoopThread();
	poller_->updateChannel(channel);
}

void EventLoop::loop()
{
	assertInLoopThread();
	assert(!looping_);
	looping_ = true;
	quit_ = false;

	while(!quit_)
	{
		ActiveChannels activeChannels;
		pollReturnTime = 
			poller_->poll(kTimeOutMs, activeChannels);

		for(auto channel : activeChannels)
		{
			channel->handleEvent();
		}
	}

	looping_ = false;
}

void EventLoop::quit()
{
	quit_ = true;
}

void EventLoop::assertInLoopThread()
{
	if(!isInLoopThread())
	{
		abortNotInLoopThread();
	}
}

bool EventLoop::isInLoopThread()
{
	return CurrentThread::tid() == pthreadId_;
}

void EventLoop::abortNotInLoopThread()
{
	LOG_ERROR << "EventLoop " << this << " created in "
		      << pthreadId_ << " CurrentThread::tid is "
			  << CurrentThread::tid();
}

TimerId EventLoop::runAt(Timestamp when, const Callback &cb)
{
	return timerQueue_->addTimer(cb, when, 0.0);
}

TimerId EventLoop::runAfter(double delay, const Callback &cb)
{
	return runAt(addTime(Timestamp::now(), delay), cb);
}

TimerId EventLoop::runEvery(double interval, const Callback &cb)
{
	return timerQueue_->addTimer(cb, 
			                     addTime(Timestamp::now(), interval),
								 interval);
}

}//muduo
