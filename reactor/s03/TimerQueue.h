#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include <utility>
#include <memory>
#include <vector>
#include <set>

#include <muduo/base/Timestamp.h>

#include "Channel.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

namespace muduo
{

class TimerQueue
{

public:
	TimerQueue(EventLoop *loop);

	~TimerQueue();

	TimerId addTimer(const TimerCallback &cb,
			         Timestamp when,
					 double interval);

private:
	typedef std::unique_ptr<Timer> PTimer;
	typedef std::pair<Timestamp, PTimer> Entry;
	typedef std::set<Entry> TimerList;
	typedef std::vector<PTimer> ExpiredTimers;

	void insert(PTimer&& timer);

	ExpiredTimers getExpired(Timestamp now);

	void reset(ExpiredTimers& expiredTimers, Timestamp now);

	void handleRead();

	EventLoop *loop_;
	const int timerFd_;
	Channel timerChannel_;
	TimerList timers_;

};//TimerQueue

}//muduo

#endif
