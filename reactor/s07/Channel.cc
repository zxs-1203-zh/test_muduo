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
	idx_(-1),
	eventHandling_(false)
{}

void Channel::update()
{
	loop_->updateChannel(this);
}

Channel::~Channel()
{
	assert(!eventHandling_);
}

void Channel::handleEvent(Timestamp receiveTime)
{
	eventHandling_ = true;

	if(revents_ & POLLNVAL)
	{
		LOG_WARN << "Channel::handleEvent() POLLNVAL";
	}

	if((revents_ & POLLHUP) && !(revents_ & POLLIN))
	{
		if(closeCallback_)
		{
			closeCallback_();
		}
	}
	else if(revents_ & kReadEvent)
	{
		if(readCallback_)
		{
			readCallback_(receiveTime);
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
	eventHandling_ = false;
}

}//muduo
