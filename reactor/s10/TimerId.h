#ifndef TIMERID_H
#define TIMERID_H

#include "Timer.h"

namespace muduo
{

class TimerId
{

public:
	TimerId(Timer *timer):
		timer_(timer)
	{}

private:
	Timer *timer_;

};//TimerId

}//muduo

#endif
