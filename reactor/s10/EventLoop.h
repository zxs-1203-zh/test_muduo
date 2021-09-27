#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <atomic>

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
	typedef std::function<void()> Functor;

	EventLoop();

	~EventLoop();

	void updateChannel(Channel *);

	void removeChannel(Channel *);

	void loop();

	void quit();

	void assertInLoopThread()
	{
		if(!isInLoopThread())
		{
			abortNotInLoopThread();
		}
	}

	TimerId runAt(Timestamp when, 
			      const Functor& cb);

	TimerId runAfter(double delay,
			         const Functor& cb);

	TimerId runEvery(double interval,
			         const Functor& cb);

	void runInLoop(const Functor& cb);

	void queueInLoop(const Functor& cb);

	bool isInLoopThread()
	{
		return pthreadId_ == CurrentThread::tid();
	}

private:
	void abortNotInLoopThread()
	{
		LOG_ERROR << "EventLoop " << this << " created in "
				  << pthreadId_ << " CurrentThread::tid is "
				  << CurrentThread::tid();
	}

	void wakeup();

	void handleRead();

	void doPendingFunctors();

	const pid_t pthreadId_;

	bool looping_;
	std::atomic_bool quit_;
	bool callingPendingFunctors_;

	std::unique_ptr<Poller> poller_;

	std::unique_ptr<TimerQueue> timerQueue_;

	int wakeupFd_;
	std::unique_ptr<Channel> wakeupChannel_;
	mutable std::mutex mutPenFunc_;
	std::vector<Functor> pendingFunctors_; //guarded by mut
	
	Timestamp pollReturnTime_;
};

}//muduo

#endif
