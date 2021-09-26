#ifndef TIMER_H
#define TIMER_H

#include <functional>

#include <muduo/base/Timestamp.h>

#include "Callbacks.h"

namespace muduo
{

class Timer//noncopyable
{

public:
	Timer(const Timer&) = delete;
	Timer& operator=(const Timer&) = delete;

	Timer(const TimerCallback& cb,
		  Timestamp when,
		  double interval):
		Callback_(cb),
		expiraton_(when),
		interval_(interval),
		repeat_(interval > 0.0)
	{}

	bool repeat()const
	{
		return repeat_;
	}

	void run()const
	{
		Callback_();
	}

	Timestamp expiraton()const
	{
		return expiraton_;
	}

	void restart(Timestamp now)
	{
		if(repeat_)
		{
			expiraton_ = addTime(now, interval_);
		}
		else
		{
			expiraton_ = Timestamp::invalid();
		}
	}


private:
	TimerCallback Callback_;
	Timestamp expiraton_;
	double interval_;
	const bool repeat_;

};//Timer

}//muduo

#endif
