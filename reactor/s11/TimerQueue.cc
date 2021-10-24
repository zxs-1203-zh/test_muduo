#include <algorithm>
#include <bits/types/struct_timespec.h>
#include <ctime>
#include <functional>
#include <memory>
#include <strings.h>
#include <sys/timerfd.h>

#include <muduo/base/Logging.h>
#include <unistd.h>
#include <utility>

#include "TimerQueue.h"


namespace muduo
{

namespace detail
{

int createTimerFd()
{
	int timerFd = ::timerfd_create(CLOCK_MONOTONIC,
			                       TFD_CLOEXEC | TFD_NONBLOCK);

	if(timerFd < 0)
	{
		LOG_SYSERR << "muduo::detail::createTimerFd()";
	}

	return timerFd;
}

timespec howMuchTimeFromNow(Timestamp when)
{
	int microSeconds = when.microSecondsSinceEpoch() -
		              Timestamp::now().microSecondsSinceEpoch();

	if(microSeconds < 100)
	{
		microSeconds = 100;
	}

	timespec ret;
	ret.tv_sec = 
	microSeconds / Timestamp::kMicroSecondsPerSecond;
	
	ret.tv_nsec =
	(microSeconds % Timestamp::kMicroSecondsPerSecond) * 1000;

	return ret;
}

void resetTimerFd(int timerFd, Timestamp expiraton)
{
	itimerspec newValue;
	::bzero(&newValue, sizeof(newValue));
	newValue.it_value = howMuchTimeFromNow(expiraton);

	if(::timerfd_settime(timerFd, 0, &newValue, nullptr) == -1)
	{
		LOG_SYSERR << "muduo::detail::resetTimerFd()";
	}
}

void readTimerFd(int timerFd)
{
	int64_t howMany;
	ssize_t n = ::read(timerFd, &howMany, sizeof howMany);

	if(n != sizeof howMany)
	{
		LOG_SYSERR << "handleRead() read " << n
			       << " bytes instead of 8";
	}
}

}//detail

TimerQueue::TimerQueue(EventLoop *loop):
	loop_(loop),
	timerFd_(detail::createTimerFd()),
	timerChannel_(loop, timerFd_),
	callingExpiredTimers_(false)
{
	timerChannel_.setReadCallback(
			std::bind(&TimerQueue::handleRead, this));

	timerChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
	::close(timerFd_);
}

TimerId TimerQueue::addTimer(const TimerCallback& cb,
		         Timestamp when,
				 double interval)
{
	Timer* newTimer(new Timer(cb, when, interval));

	loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop,
			         this,
					 newTimer));

	return TimerId(newTimer, newTimer->sequence());
}

void TimerQueue::cancel(TimerId timerId)
{
	loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer* newTimer)
{
	loop_->assertInLoopThread();

	insert(PTimer(newTimer));
}

TimerQueue::ExpiredTimers TimerQueue::getExpired(Timestamp now)
{
	loop_->assertInLoopThread();
	assert(timers_.size() == activeTimers_.size());

	ExpiredTimers expiredTimers;

	for(auto it = timers_.begin(); it != timers_.end();)
	{
		if(it->first <= now)
		{
			expiredTimers.push_back(
					std::move(const_cast<PTimer&>(it->second)));
			it = timers_.erase(it);
		}
		else
		{
			++it;
		}
	}

	for(auto &timer : expiredTimers)
	{
		size_t n = activeTimers_.erase(TimerId(timer.get(), timer->sequence()));
		assert(n == 1);
		(void)n;
	}

	assert(timers_.size() == activeTimers_.size());

	return expiredTimers;
}

void TimerQueue::reset(ExpiredTimers& expiredTimers,
		               Timestamp now)
{
	loop_->assertInLoopThread();

	for(auto &timer : expiredTimers)
	{
		TimerId timerId(timer.get(), timer->sequence());
		if(timer->repeat() && cancelingTimers_.find(timerId) == cancelingTimers_.end())
		{
			timer->restart(now);
			insert(std::move(timer));
		}
	}

	cancelingTimers_.clear();
}

void TimerQueue::handleRead()
{
	loop_->assertInLoopThread();

	Timestamp now(Timestamp::now());

	ExpiredTimers expiredTimers = getExpired(now);

	callingExpiredTimers_ = true;

	for(auto &timer : expiredTimers)
	{
		timer->run();
	}

	callingExpiredTimers_ = false;

	reset(expiredTimers, now);
}

void TimerQueue::insert(PTimer&& timer)
{
	loop_->assertInLoopThread();
	assert(timers_.size() == activeTimers_.size());

	auto it = timers_.begin();

	auto when = timer->expiraton();

	if(it == timers_.end() || it->first > when)
	{
		detail::resetTimerFd(timerFd_, when);
	}

	{
	auto result = activeTimers_.insert(
			TimerId(timer.get(), timer->sequence()));
	assert(result.second);
	(void)result;
	}

	{
	auto result = timers_.insert(
			std::make_pair(when, std::move(timer)));
	assert(result.second);
	(void)result;
	}

	assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
	loop_->assertInLoopThread();
	assert(timers_.size() == activeTimers_.size());

	auto it = activeTimers_.find(timerId);

	if(it != activeTimers_.end())
	{
		auto itTimers = timers_.begin();
		for(; itTimers != timers_.end(); ++itTimers)
		{
			if(itTimers->first == it->timer_->expiraton() && 
			   itTimers->second.get() == it->timer_)
			{
				break;
			}
		}
		assert(itTimers != timers_.end());
		timers_.erase(itTimers);

		activeTimers_.erase(it);
	}
	else if(callingExpiredTimers_)
	{
		cancelingTimers_.insert(timerId);
	}

	assert(timers_.size() == activeTimers_.size());
}


}//muduo
