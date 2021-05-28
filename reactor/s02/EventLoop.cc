#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>

#include "Poller.h"
#include "Channel.h"
#include "EventLoop.h"


namespace muduo
{

__thread EventLoop* t_loopInThread;

const int EventLoop::kTimeoutMs = 10000;

EventLoop::EventLoop():
	poller_(new Poller(this)),
	looping_(false),
	quit_(false),
	threadId_(CurrentThread::tid())
{
	LOG_TRACE << "EventLoop created " << this << " in thread " << t_loopInThread;
	if(t_loopInThread)
	{
		LOG_FATAL << "Another EventLoop " << t_loopInThread
			      << " exists in this thread " << CurrentThread::tid();
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

}//muduo
