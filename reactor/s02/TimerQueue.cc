#include <algorithm>
#include <functional>
#include <memory>
#include <sys/timerfd.h>
#include <utility>

#include "TimerQueue.h"
#include "EventLoop.h"

//for debug
#include <stdio.h>
#include <typeinfo>


namespace muduo
{

namespace detail
{
	int createTimerFd()
	{
		int timerFd = ::timerfd_create(CLOCK_MONOTONIC,
				                      TFD_NONBLOCK | 
									  TFD_CLOEXEC);

		if(timerFd < 0)
		{
			LOG_SYSERR << "::timerfd_create()";
		}

		return timerFd;
	}

	struct timespec howMuchTimeFromNow(Timestamp when)
	{
		struct timespec ts;

		int microSeconds =  when.microSecondsSinceEpoch() - 
			Timestamp::now().microSecondsSinceEpoch();

		if(microSeconds < 100)
		{
			microSeconds = 100;
		}

		ts.tv_sec = static_cast<time_t>(microSeconds / 
			Timestamp::kMicroSecondsPerSecond);

		ts.tv_nsec = static_cast<long>((microSeconds %
			Timestamp::kMicroSecondsPerSecond) * 1000);

		return ts;
	}

	void readTimerFd(int timerFd, Timestamp now)
	{
		uint64_t howMany;
		ssize_t n = ::read(timerFd, &howMany, sizeof(howMany));

		LOG_TRACE << "TimerQueue::handleRead() "
			      << howMany
				  << " at "
				  << now.toFormattedString();

		if(n != sizeof(howMany))
		{
			LOG_ERROR << "TimerQueue::handleRead() "
				      << "reads " << n << " bytes "
					  << "insted of 8";
		}
	}

	void resetTimerFd(int timerFd, Timestamp expiration)
	{
		struct itimerspec newValue;

		::bzero(&newValue, sizeof(newValue));
		newValue.it_value = howMuchTimeFromNow(expiration);
		int ret = ::timerfd_settime(timerFd, 0, 
									&newValue, NULL);

		if(ret)
		{
			LOG_SYSERR << "::timerfd_settime()";
		}
	}

}//detail

TimerQueue::TimerQueue(EventLoop *loop):
	loop_(loop),
	timerFd_(detail::createTimerFd()),
	channel_(loop, timerFd_)
{
	printf("TimerQueue::TimerQueue()\n");

	channel_.setReadCallback(
			std::bind(&TimerQueue::handleRead, this));

	channel_.enableReading();
}

TimerQueue::~TimerQueue()
{
	::close(timerFd_);
}

TimerId TimerQueue::addTimer(const TimerCallback &cb,
		                     Timestamp when,
							 double interval)
{
	loop_->assertInLoopThread();

	std::unique_ptr<Timer> timer(new Timer(cb, when, interval));

	TimerId timerId(timer.get());

	insert(std::move(timer));

	return timerId;
}

void TimerQueue::handleRead()
{
	loop_->assertInLoopThread();

	Timestamp now(Timestamp::now());

	detail::readTimerFd(timerFd_, now);

	auto expiredTimers = getExpired(now);

	for(auto &expired : expiredTimers)
	{
		expired->run();
	}

	reset(expiredTimers, now);
}

void TimerQueue::reset(expiredList& expiredTimers,
		               Timestamp now)
{
	for(auto &expired : expiredTimers)
	{
		if(expired->repeat())
		{
			expired->restart(now);
			insert(std::move(expired));
		}
	}

	if(!timers_.empty())
	{
		auto it = timers_.begin();
		detail::resetTimerFd(timerFd_, it->second->expiration());
	}
}

TimerQueue::expiredList TimerQueue::getExpired(Timestamp now)
{
	expiredList expiredTimers;
	auto it = timers_.begin();

	for(; it != timers_.end(); )
	{
		if(it->first < now)
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

void TimerQueue::insert(std::unique_ptr<Timer> &&timer)
{
	Timestamp when = timer->expiration();
	
	auto it = timers_.begin();
	if(it == timers_.end() || it->first > when)
	{
		detail::resetTimerFd(timerFd_, when);
	}

	timers_.insert(std::make_pair(when, std::move(timer)));

}

}//muduo
