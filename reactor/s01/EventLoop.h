#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include <memory>


namespace muduo
{

class Channel;
class Poller;

class EventLoop
{
public:
	EventLoop();

	~EventLoop();

	void loop();

	void quit();

	void updateChannel(Channel*);

	void asserInLoopThread();

private:
	typedef std::vector<Channel*> ChannelList;

	void abortNotInLoopThread();

	bool looping_;
	bool quit_;
	const pid_t threadId_;
	std::unique_ptr<Poller> poller_;
	ChannelList activeChannels_;
};//EventLoop

}//muduo

#endif
