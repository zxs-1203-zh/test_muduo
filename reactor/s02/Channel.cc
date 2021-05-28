#include <poll.h>
#include <unistd.h>

#include "EventLoop.h"
#include "Channel.h"

namespace muduo
{

const int Channel::kReadEvent = (POLLIN | POLLPRI);
const int Channel::kWriteEvent = POLLOUT;
const int Channel::kErrorEvent = (POLLNVAL | POLLHUP);

Channel::Channel(EventLoop *loop, int fd):
	loop_(loop),
	fd_(fd),
	events_(0),
	revents_(0),
	idx_(-1)
{}

Channel::~Channel()
{
	::close(fd_);
}

void Channel::update()
{
	loop_->updateChannel(this);
}

void Channel::handleEvent()
{
	if(revents_ & POLLNVAL)
	{
		LOG_WARN << "Channel::handleEvent() POLLNVAL";
	}

	if(revents_ & (POLLIN | POLLPRI | POLLRDHUP))
	{
		if(readCallback_)
		{
			readCallback_();
		}
	}
	else if(revents_ & POLLOUT)
	{
		if(writeCallback_)
		{
			writeCallback_();
		}
	}
	else if(revents_ & (POLLHUP | POLLNVAL))
	{
		if(errorCallback_)
		{
			errorCallback_();
		}
	}
}

}//muduo
