#ifndef POLLER_H
#define POLLER_H

#include <vector>
#include <map>
#include <poll.h>

#include <muduo/base/Timestamp.h>

namespace muduo
{

class EventLoop;
class Channel;
	
typedef std::vector<Channel*> ActiveChannels;

class Poller
{

public:
	Poller(EventLoop *loop);

	Timestamp poll(int timeOutMs, ActiveChannels& activeChannels);

	void updateChannel(Channel* channel);

	void removeChannel(Channel* channel);

private:
	typedef std::map<int, Channel*> ChannelList;
	typedef std::vector<struct pollfd> PollFdList;

	void findActiveChannels(int numEvents,
			                ActiveChannels &activeChannels);

	EventLoop *loop_;
	PollFdList pollFds_;
	ChannelList channels_;

};//Poller

}//muduo

#endif
