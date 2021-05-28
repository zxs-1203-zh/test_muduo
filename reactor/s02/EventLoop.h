#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <memory>
#include <vector>

namespace muduo
{

class Channel;
class Poller;

class EventLoop
{

public:
	typedef std::vector<Channel *> ChannelList;

	EventLoop();
	~EventLoop();

	void loop();

	void quit();

	void updateChannel(Channel *);

	void assertInLoopThread();
private:
	void abortNotInLoopThread();

	std::unique_ptr<Poller> poller_;
	bool looping_;
	bool quit_;
	const pid_t threadId_;
	ChannelList activeChannels_;

	static const int kTimeoutMs;

};//EventLoop


}//muduo

#endif
