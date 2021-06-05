#ifndef TIMER_QUEUE_H
#define TIMER_QUEUE_H

#include <functional>
#include <memory>
#include <utility>
#include <set>

#include <muduo/base/Timestamp.h>
#include <vector>

#include "Timer.h"
#include "TimerId.h"
#include "Channel.h"

namespace muduo
{

class EventLoop;

class TimerQueue//noncopyable
{

public:
	typedef std::function<void()> TimerCallback;

	TimerQueue(const TimerQueue&) = delete;
	TimerQueue& operator=(const TimerQueue&) = delete;

	TimerQueue(EventLoop *loop);
	~TimerQueue();

	TimerId addTimer(const TimerCallback &cb,
			         Timestamp expiration,
					 double interval);

private:
	typedef std::unique_ptr<Timer> PTimer;
	typedef std::pair<Timestamp, PTimer> Entry;
	typedef std::set<Entry> TimerList;
	typedef std::vector<PTimer> expiredList;

	void handleRead();

	void insert(PTimer &&timer);

	expiredList getExpired(Timestamp now);

	void reset(expiredList& expiredTimers, Timestamp now);

	EventLoop *loop_;
	const int timerFd_;
	Channel channel_;
	TimerList timers_;

};//TimerQueue

}//muduo

#endif
