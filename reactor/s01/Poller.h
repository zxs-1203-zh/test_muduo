#ifndef POLLER_H
#define POLLER_H

#include <poll.h>
#include <vector>
#include <map>

#include <muduo/base/Timestamp.h>

#include "EventLoop.h"
#include "Channel.h"

namespace muduo
{

class Poller //noncopyable
{
public:
	typedef std::vector<Channel *> ChannelList;

	Poller(const Poller&) = delete;
	Poller& operator=(const Poller&) = delete;

	Poller(EventLoop *loop);

	Timestamp poll(int timeoutMs, ChannelList *activeChannels);

	void updateChannel(Channel *channel);

	void asserInLoopThread()
	{
		loop_->asserInLoopThread();
	}

private:
	void fillActiveChannels(int numEvents,
			                ChannelList *activeChannels);
	
	typedef std::vector<struct pollfd> PollFdList;
	typedef std::map<int, Channel*> ChannelMap;

	EventLoop *loop_;
	PollFdList pollfds_;
	ChannelMap channels_;

};//Poller

}//muduo

#endif
