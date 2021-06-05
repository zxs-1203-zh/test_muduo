#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>

#include "Poller.h"
#include "Channel.h"
#include "TimerId.h"
#include "TimerQueue.h"
#include "EventLoop.h"


namespace muduo
{

__thread EventLoop* t_loopInThread;

const int EventLoop::kTimeoutMs = 10000;

EventLoop::EventLoop():
	poller_(new Poller(this)),
	looping_(false),
	quit_(false),
	threadId_(CurrentThread::tid()),
	timeQueue_(new TimerQueue(this))
{
	LOG_TRACE << "EventLoop created " << this 
		      << " in thread " << t_loopInThread;

	if(t_loopInThread)
	{
		LOG_FATAL << "Another EventLoop " << t_loopInThread
			      << " exists in this thread " 
				  << CurrentThread::tid();
	}
	else
	{
		t_loopInThread = this;
	}
}

EventLoop::~EventLoop()
{
	assert(!looping_);
	t_loopInThread = nullptr;
}

void EventLoop::loop()
{
	assert(!looping_);
	assertInLoopThread();
	looping_ = true;
	quit_ = false;

	while(!quit_)
	{
		activeChannels_.clear();
		pollReturnTime =
			poller_->poll(activeChannels_, kTimeoutMs);

		for(auto &channel : activeChannels_)
		{
			channel->handleEvent();
		}
	}
	LOG_TRACE << "EventLoop " << this << " stop looping";

	looping_ = false;
}

void EventLoop::quit()
{
	quit_ = true;
}

void EventLoop::updateChannel(Channel *channel)
{
	assertInLoopThread();
	poller_->updateChannel(channel);
}

void EventLoop::assertInLoopThread()
{
	if(threadId_ != CurrentThread::tid())
	{
		abortNotInLoopThread();
	}
}

void EventLoop::abortNotInLoopThread()
{
	LOG_FATAL << "EventLoop::abortNotInLoopThread() - EventLoop "
		      << this << " was created in thread " << threadId_
			  << " CurrentThread is " << CurrentThread::tid();
}

TimerId EventLoop::runAt(const Timestamp when,
		      			 const TimerCallback &cb)
{
	return timeQueue_->addTimer(cb, when, 0.0);
}

TimerId EventLoop::runAfter(double delay, 
		                    const TimerCallback &cb)
{
	return runAt(addTime(Timestamp::now(), delay), cb);
}

TimerId EventLoop::runEvery(double interval,
		                    const TimerCallback &cb)
{
	return timeQueue_->addTimer(
			cb,
			addTime(Timestamp::now(), interval),
			interval
			);
}

}//muduo
