#ifndef TIMERID_H
#define TIMERID_H

#include "Timer.h"

namespace muduo
{

class TimerId //copyable
{

public:
	explicit TimerId(Timer *time): time_(time)
	{}

private:
	Timer *time_;

};//TimerId

}//muduo


#endif
