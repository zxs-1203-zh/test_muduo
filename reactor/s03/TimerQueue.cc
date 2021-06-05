#include <algorithm>
#include <bits/types/struct_timespec.h>
#include <ctime>
#include <functional>
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
	timerChannel_(loop, timerFd_)
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
	PTimer newTimer(new Timer(cb, when, interval));

	TimerId ret(newTimer.get());


	loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop,
			         this,
					 std::ref(newTimer)));

	return ret;
}

void TimerQueue::addTimerInLoop(PTimer& newTimer)
{
	loop_->assertInLoopThread();

	insert(std::move(newTimer));
}

TimerQueue::ExpiredTimers TimerQueue::getExpired(Timestamp now)
{
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

	return expiredTimers;
}

void TimerQueue::reset(ExpiredTimers& expiredTimers,
		               Timestamp now)
{
	for(auto &timer : expiredTimers)
	{
		if(timer->repeat())
		{
			timer->restart(now);
			insert(std::move(timer));
		}
	}
}

void TimerQueue::handleRead()
{
	loop_->assertInLoopThread();

	Timestamp now(Timestamp::now());

	ExpiredTimers expiredTimers = getExpired(now);

	for(auto &timer : expiredTimers)
	{
		timer->run();
	}

	reset(expiredTimers, now);
}

void TimerQueue::insert(PTimer&& timer)
{
	auto it = timers_.begin();

	if(it == timers_.end() || it->first > timer->expiraton())
	{
		detail::resetTimerFd(timerFd_, timer->expiraton());
	}

	timers_.insert(
			std::make_pair(timer->expiraton(), std::move(timer)));
}


}//muduo
