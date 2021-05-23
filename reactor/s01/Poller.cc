#include <muduo/base/Logging.h>

#include "Poller.h"


namespace muduo
{

Poller::Poller(EventLoop *loop):
	loop_(loop)
{}

Timestamp Poller::poll(int timeoutMs, 
					   ChannelList *activeChannels)
{
	int numEvents = ::poll(pollfds_.data(),
			               pollfds_.size(),
						   timeoutMs);

	Timestamp now(Timestamp::now());

	if(numEvents > 0)
	{
		LOG_TRACE << numEvents << " events happended";
		fillActiveChannels(numEvents, activeChannels);
	}
	else if(numEvents == 0)
	{
		LOG_TRACE << "nothing happended";
	}
	else
	{
		LOG_SYSERR << "Poller::poll()";
	}

	return now;
}

void Poller::updateChannel(Channel *channel)
{
	asserInLoopThread();
	LOG_TRACE << "fd: " << channel->fd() << "events: " << channel->events();

	if(channel->index() < 0)
	{
		assert(channels_.find(channel->fd()) == channels_.end());

		pollfd pfd;
		pfd.fd = channel->fd();
		pfd.events = channel->events();
		pfd.revents = 0;

		pollfds_.push_back(pfd);
		channel->set_index(pollfds_.size() - 1);
		channels_[pfd.fd] = channel;
	}
	else
	{
		assert(channels_.find(channel->fd()) != channels_.end());

		auto idx = channel->index();

		assert(idx >= 0 && idx < pollfds_.size());
		assert(pollfds_[idx].fd == channel->fd() || pollfds_[idx].fd == -1);

		pollfds_[idx].events = channel->events();
		pollfds_[idx].revents = 0;

		if(channel->isNoneEvent())
		{
			pollfds_[idx].fd = -1;
		}
	}
}

void Poller::fillActiveChannels(int numEvents,
		                        ChannelList *activeChannels)
{
	for(auto& pollfd : pollfds_)
	{
		if(pollfd.revents)
		{
			auto it = channels_.find(pollfd.fd);

			assert(it != channels_.end());
			assert(it->first == it->second->fd());

			it->second->set_revents(pollfd.revents);

			activeChannels->push_back(it->second);

			if(--numEvents <= 0)
			{
				break;
			}
		}
	}
}

}//muduo
