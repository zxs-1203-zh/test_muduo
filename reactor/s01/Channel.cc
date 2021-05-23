#include "Channel.h"
#include "EventLoop.h"
#include <sys/poll.h>
#include <muduo/base/Logging.h>


namespace muduo
{

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fdArgs)
	:loop_(loop),
	 fd_(fdArgs),
	 events_(0),
	 revents_(0),
	 idx_(-1)
{ }

void Channel::handleEvent()
{
	if(revents_ & POLLNVAL)
	{
		LOG_WARN << "Channel::handleEvent() POLLNVAL";
	}

	if(revents_ & (POLLNVAL | POLLERR))
	{
		if(errorEventCallback_)
		{
			errorEventCallback_();
		}
	}

	if(revents_ & (POLLIN | POLLPRI | POLLRDHUP))
	{
		if(readEventCallback_)
		{
			readEventCallback_();
		}
	}

	if(revents_ & POLLOUT)
	{
		if(writeEventCallback_)
		{
			writeEventCallback_();
		}
	}

}

void Channel::update()
{
	loop_->updateChannel(this);
}

}//muduo
