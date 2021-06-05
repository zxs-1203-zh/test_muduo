#include <poll.h>

#include <muduo/base/Logging.h>

#include "EventLoop.h"
#include "Channel.h"

namespace muduo
{

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI | POLLRDHUP;
const int Channel::kWriteEvent = POLLOUT;
const int Channel::kErrorEvent = POLLNVAL | POLLERR;

Channel::Channel(EventLoop *loop, int fd):
	loop_(loop), 
	fd_(fd),
	events_(0),
	revents_(0),
	idx_(-1)
{}

void Channel::update()
{
	loop_->updateChannel(this);
}

void Channel::handleEvent()
{
	if(revents_ & kReadEvent)
	{
		if(readCallback_)
		{
			readCallback_();
		}
	}
	else if(revents_ & kWriteEvent)
	{
		if(writeCallback_)
		{
			writeCallback_();
		}
	}
	else if(revents_ & kErrorEvent)
	{
		if(errorCallback_)
		{
			errorCallback_();
		}
	}
}

}//muduo
