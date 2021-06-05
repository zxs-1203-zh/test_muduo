#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <memory>
#include <vector>
#include <functional>

#include <muduo/base/Timestamp.h>

#include "TimerId.h"

namespace muduo
{

class Channel;
class Poller;
class TimerQueue;

class EventLoop
{

public:
	typedef std::vector<Channel *> ChannelList;
	typedef std::function<void()> TimerCallback;

	EventLoop();
	~EventLoop();

	void loop();

	void quit();

	void updateChannel(Channel *);

	void assertInLoopThread();

	TimerId runAt(const Timestamp when, 
			      const TimerCallback& cb);

	TimerId runAfter(double delay, const TimerCallback &cb);

	TimerId runEvery(double interval, const TimerCallback &cb);

private:
	void abortNotInLoopThread();

	std::unique_ptr<Poller> poller_;
	bool looping_;
	bool quit_;
	const pid_t threadId_;
	ChannelList activeChannels_;
	std::unique_ptr<TimerQueue> timeQueue_;
	Timestamp pollReturnTime;

	static const int kTimeoutMs;

};//EventLoop


}//muduo

#endif
