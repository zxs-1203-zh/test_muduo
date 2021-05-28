#ifndef TIMERID_H
#define TIMERID_H

#include "Timer.h"

namespace muduo
{

class TimerId //noncopyable
{

public:
	TimerId(const TimerId&) = delete;
	TimerId& operator=(const TimerId&) = delete;

	TimerId(Timer *time): time_(time)
	{}

private:
	Timer *time_;

};//TimerId

}//muduo


#endif
