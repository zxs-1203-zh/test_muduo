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

	TimerId ret(newTimer, newTimer->sequence());


	loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop,
			         this,
					 newTimer));

	return ret;
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

void TimerQueue::cancelInLoop(TimerId timerId)
{
	loop_->assertInLoopThread();
	assert(timers_.size() == activeTimers_.size());

	auto it = activeTimers_.find(timerId);

	if(it != activeTimers_.end())
	{
		activeTimers_.erase(it);
		Timer *timer = new Timer(nullptr, Timestamp::invalid(), 0);
		const_cast<int64_t&>(timer->sequence_) = timerId.sequence_;
		Entry newEntry = std::make_pair(timerId.timer_->expiraton(), PTimer(timer));
		auto itNewEntry = timers_.find(newEntry);
		assert(itNewEntry != timers_.end() && itNewEntry->second.get() == timerId.timer_);
		timers_.erase(itNewEntry);
	}
	else if(callingExpiredTimers_)
	{
		cancelingTimers_.insert(timerId);
	}

	assert(timers_.size() == activeTimers_.size());
}

TimerQueue::ExpiredTimers TimerQueue::getExpired(Timestamp now)
{
	assert(timers_.size() == activeTimers_.size());
	ExpiredTimers expiredTimers;

	for(auto it = timers_.begin(); it != timers_.end();)
	{
		if(it->first <= now)
		{
			TimerId timerId(it->second.get(), it->second->sequence());
			size_t n = activeTimers_.erase(timerId);
			assert(n == 1);
			(void)n;
			expiredTimers.push_back(
					std::move(const_cast<PTimer&>(it->second)));
			it = timers_.erase(it);
		}
		else
		{
			++it;
		}
	}
	assert(timers_.size() == activeTimers_.size());

	return expiredTimers;
}

void TimerQueue::reset(ExpiredTimers& expiredTimers,
		               Timestamp now)
{
	for(auto &timer : expiredTimers)
	{
		if(timer->repeat() &&
		   cancelingTimers_.find(TimerId(timer.get(), timer->sequence())) == cancelingTimers_.end())
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
	assert(timers_.size() == activeTimers_.size());
	auto it = timers_.begin();

	auto when = timer->expiraton();

	if(it == timers_.end() || it->first > when)
	{
		detail::resetTimerFd(timerFd_, when);
	}

	{
		TimerId timerId(timer.get(), timer->sequence());
		auto result = activeTimers_.insert(timerId);
		assert(result.second);
		(void)result;
	}
	{
		auto result = timers_.insert(std::make_pair(when, std::move(timer)));
		assert(result.second);
		(void)result;
	}

	assert(timers_.size() == activeTimers_.size());
}


}//muduo
