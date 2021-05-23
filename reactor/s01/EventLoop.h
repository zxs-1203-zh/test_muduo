#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "Channel.h"

namespace muduo
{

class EventLoop
{
public:
	void updateChannel(Channel*);

	void asserInLoopThread();

private:
};//EventLoop

}//muduo

#endif
