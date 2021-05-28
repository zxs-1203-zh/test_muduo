#ifndef POLL_H
#define POLL_H

#include <vector>
#include <map>
#include <poll.h>

#include<muduo/base/Timestamp.h>


namespace muduo
{
class Channel;
class EventLoop;

class Poller //noncopyable
{

public:
	Poller(const Poller&) = delete;
	Poller& operator=(const Poller&) = delete;

	Poller(EventLoop *loop);

	typedef std::vector<Channel*> ChannelList;

	Timestamp poll(ChannelList &activeChannels, int timeOutMs);

	void updateChannel(Channel *);

private:
	typedef std::vector<struct pollfd> PollfdList;
	typedef std::map<int, Channel*> ChannelMap;

	void fillActiveChannels(int numEvents, ChannelList& avtiveChannels);

	EventLoop *ownerLoop_;
	PollfdList pollfds_;
	ChannelMap channels_;

};//Poller

}//muduo


#endif
