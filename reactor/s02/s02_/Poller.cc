#include <algorithm>

#include <poll.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>

#include "Channel.h"
#include "Poller.h"


namespace muduo
{

Poller::Poller(EventLoop *loop):
	ownerLoop_(loop)
{}

Timestamp Poller::poll(ChannelList &activeChannels, int timeOutMs)
{
	const int numEvents = ::poll(pollfds_.data(), pollfds_.size(), timeOutMs);
	Timestamp now(Timestamp::now());

	if(numEvents > 0)
	{
		LOG_TRACE << numEvents << " events happened";
		fillActiveChannels(numEvents, activeChannels);
	}
	else if(numEvents == 0)
	{
		LOG_TRACE << "nothing happended";
	}
	else
	{
		LOG_SYSERR << " Poller::poll()";
	}

	return now;
}

void Poller::updateChannel(Channel *channel)
{
	LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->event();
	if(channel->index() == -1)
	{
		assert(channels_.find(channel->fd()) == channels_.end());
		struct pollfd pfd;
		pfd.fd = channel->fd();
		if(channel->isNonEvent())
		{
			pfd.fd = -1;
		}
		pfd.events = channel->event();
		pfd.revents = 0;
		pollfds_.push_back(std::move(pfd));
		channel->setIndex(pollfds_.size() - 1);
		channels_[pfd.fd] = channel;
	}
	else
	{
		int idx = channel->index();
		assert(channels_.find(channel->fd()) != channels_.end());
		assert(channels_[channel->fd()] == channel);
		assert(idx >= 0 && idx < pollfds_.size());
		auto& pfd = pollfds_[idx];
		assert(pfd.fd == channel->fd() || pfd.fd == -1);
		pfd.events = channel->event();
		pfd.revents = 0;
		if(channel->isNonEvent())
		{
			pfd.fd = -1;
		}
	}
}

void Poller::fillActiveChannels(int numEvents, ChannelList& activeChannels)
{
	for(auto pfd : pollfds_)
	{
		if(pfd.revents != 0)
		{
			auto it = channels_.find(pfd.fd);
			assert(it != channels_.end());
			assert(it->first == it->second->fd());
			it->second->setRevent(pfd.revents);
			activeChannels.push_back(it->second);
			//pfd.revents = 0;
			if(--numEvents == 0)
			{
				break;
			}
		}
	}
}

}//muduo
