#include <atomic>
#include <mutex>
#include <sys/eventfd.h>

#include <muduo/base/Logging.h>
#include <unistd.h>

#include "TimerQueue.h"
#include "Channel.h"
#include "Poller.h"
#include "EventLoop.h"

namespace muduo
{

__thread EventLoop *owerLoop;

const int kTimeOutMs = 10000;

static int createEventFd()
{
	int evtFd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);

	if(evtFd < 0)
	{
		LOG_SYSERR << "EventLoop.cc:createEventFd()";
		abort();
	}

	return evtFd;
}

EventLoop::EventLoop():
	pthreadId_(CurrentThread::tid()),
	looping_(false),
	quit_(false),
	callingPendingFunctors_(false),
	poller_(new Poller(this)),
	timerQueue_(new TimerQueue(this)),
	wakeupFd_(createEventFd()),
	wakeupChannel_(new Channel(this, wakeupFd_))
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


	wakeupChannel_->setReadCallback(
			std::bind(&EventLoop::handleRead,
			          this));

	wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
	assert(!looping_);
	::close(wakeupFd_);
	owerLoop = nullptr;
}

void EventLoop::updateChannel(Channel *channel)
{
	assertInLoopThread();
	poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
	assertInLoopThread();
	poller_->removeChannel(channel);
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
		pollReturnTime_ = 
			poller_->poll(kTimeOutMs, activeChannels);

		for(auto channel : activeChannels)
		{
			channel->handleEvent();
		}

		doPendingFunctors();
	}

	looping_ = false;
}

void EventLoop::quit()
{
	quit_ = true;
	if(!isInLoopThread())
	{
		wakeup();
	}
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

TimerId EventLoop::runAt(Timestamp when, const Functor &cb)
{
	return timerQueue_->addTimer(cb, when, 0.0);
}

TimerId EventLoop::runAfter(double delay, const Functor &cb)
{
	return runAt(addTime(Timestamp::now(), delay), cb);
}

TimerId EventLoop::runEvery(double interval, const Functor &cb)
{
	return timerQueue_->addTimer(cb, 
			                     addTime(Timestamp::now(), interval),
								 interval);
}

void EventLoop::runInLoop(const Functor& cb)
{
	if(isInLoopThread())
	{
		cb();
	}
	else
	{
		queueInLoop(cb);
	}
}

void EventLoop::queueInLoop(const Functor& cb)
{
	{
		std::lock_guard<std::mutex> lk(mutPenFunc_);
		pendingFunctors_.push_back(cb);
	}

	if(!isInLoopThread() || callingPendingFunctors_)
	{
		wakeup();
	}
}

void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = ::write(wakeupFd_, &one, sizeof one);

	if(n != sizeof one)
	{
		LOG_ERROR << "EventLoop::wakeup() writes "
			      << n << " bytes instead of 8"
				  << " errno = " << errno
				  << " fd_= " << wakeupFd_;
	}
}

void EventLoop::handleRead()
{
	uint64_t one = 1;
	ssize_t n = ::read(wakeupFd_, &one, sizeof one);

	if(n != sizeof one)
	{
		LOG_ERROR << "EventLoop::handleRead() reads "
			      << n << " bytes instead of 8";
	}
}


void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;

	callingPendingFunctors_ = true;

	{
		std::lock_guard<std::mutex> lk(mutPenFunc_);
		functors.swap(pendingFunctors_);
	}

	for(auto& functor: functors)
	{
		functor();
	}

	callingPendingFunctors_ = false;
}

}//muduo
