#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <functional>
#include <memory>

#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>

namespace muduo
{

class TimerId;
class Channel;
class Poller;
class TimerQueue;

class EventLoop
{

public:
	typedef std::function<void()> Callback;

	EventLoop();

	~EventLoop();

	void updateChannel(Channel *);

	void loop();

	void quit();

	void assertInLoopThread();

	TimerId runAt(Timestamp when, 
			      const Callback& cb);

	TimerId runAfter(double delay,
			         const Callback& cb);

	TimerId runEvery(double interval,
			         const Callback& cb);

private:
	bool isInLoopThread();
	void abortNotInLoopThread();

	const pid_t pthreadId_;
	bool looping_;
	bool quit_;
	std::unique_ptr<Poller> poller_;
	Timestamp pollReturnTime;
	std::unique_ptr<TimerQueue> timerQueue_;
};

}//muduo

#endif
