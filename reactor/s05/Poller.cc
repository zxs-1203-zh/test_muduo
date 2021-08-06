#include <algorithm>
#include <cassert>
#include <muduo/base/Logging.h>

#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"

namespace muduo
{

Poller::Poller(EventLoop *loop):
	loop_(loop)
{}

Timestamp Poller::poll(int timeOutMs, 
		          ActiveChannels& activeChannels)
{
	int numEvents = 
		::poll(pollFds_.data(), pollFds_.size(), timeOutMs);

	Timestamp now(Timestamp::now());


	if(numEvents > 0){
		LOG_TRACE << numEvents << " things happended";

		Timestamp now(Timestamp::now()); 
		findActiveChannels(numEvents, activeChannels);
	}
	else if(numEvents == 0)
	{
		LOG_TRACE << "Nothing happended";
	}
	else
	{
		LOG_SYSERR << "Poller::poll()";
	}

	return now;
}

void Poller::updateChannel(Channel *channel)
{
	//FIXME
	auto it = channels_.find(channel->fd());
	int idx = channel->index();

	if(idx < 0)
	{
		assert(it == channels_.end());

		struct pollfd pfd;

		pfd.fd = channel->fd();
		pfd.events = channel->events();
		pfd.revents = 0;

		if(channel->isNoneEvents())
		{
			pfd.fd = -1;
		}

		pollFds_.push_back(std::move(pfd));

		channel->setIndex(pollFds_.size() - 1);

		channels_[channel->fd()] = channel;
	}
	else
	{
		assert(it != channels_.end());
		assert(it->second == channel);
		assert(idx >= 0 && idx < pollFds_.size());

		auto& pfd = pollFds_[idx];

		assert(pfd.fd == channel->fd() ||
			   pfd.fd == -1);

		if(!channel->isNoneEvents())
		{
			pfd.fd = channel->fd();
		}

		pfd.events = channel->events();
		pfd.revents = 0;
	}
}

void Poller::findActiveChannels(int numEvents,
		                        ActiveChannels& activeChannels)
{
	for(auto &pfd : pollFds_)
	{
		if(pfd.revents != 0)
		{
			auto it = channels_.find(pfd.fd);

			assert(it != channels_.end());
			assert(it->first == it->second->fd());

			it->second->setRevents(pfd.revents);

			activeChannels.push_back(it->second);

			if(--numEvents == 0)
			{
				break;
			}
		}
	}
}

}//muduo
