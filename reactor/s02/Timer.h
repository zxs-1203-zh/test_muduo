#ifndef TIMER_H
#define TIMER_H

#include <functional>

#include <muduo/base/Timestamp.h>

namespace muduo
{

class Timer
{

public:
	typedef std::function<void()> TimerCallback;

	Timer(const TimerCallback& cb, Timestamp when, double interval):
		callback_(cb), 
		expiration_(when),
		interval_(interval),
		repeat_(interval > 0.0)
	{}

	void run()const
	{
		callback_();
	}

	Timestamp expiration()const
	{
		return expiration_;
	}

	bool repeat()const
	{
		return repeat_;
	}

	void restart(Timestamp now);

private:
	TimerCallback callback_;
	Timestamp expiration_;
	const double interval_;
	const bool repeat_;

};//Timer

}//muduo

#endif
