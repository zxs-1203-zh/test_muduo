#ifndef TIMERID_H
#define TIMERID_H

#include "Timer.h"

namespace muduo
{

class TimerId//copyable
{

public:
	TimerId(Timer *timer = nullptr, int sequence = 0):
		timer_(timer),
		sequence_(sequence)
	{ }

	friend class TimerQueue;

	friend bool operator<(const TimerId &lhs, const TimerId &rhs);

private:
	Timer *timer_;
	int64_t sequence_;

};//TimerId

inline bool operator<(const TimerId &lhs, const TimerId &rhs)
{
	return lhs.timer_ < rhs.timer_ ||
		   (lhs.timer_ == rhs.timer_ && lhs.sequence_ < rhs.sequence_);
}

}//muduo

#endif
