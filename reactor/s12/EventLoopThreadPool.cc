#include "EventLoopThreadPool.h"
#include <memory>
#include <utility>

using namespace muduo;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop):
	baseLoop_(baseLoop),
	started_(false),
	next_(0),
	numThreads_(0)
{ }

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start()
{
	baseLoop_->assertInLoopThread();
	assert(!started_);
	started_ = true;

	for(int i = 0; i < numThreads_; ++i)
	{
		std::unique_ptr<EventLoopThread> thread(new EventLoopThread());
		threads_.push_back(std::move(thread));
		loops_.push_back(threads_[i]->startLoop());
	}
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
	baseLoop_->assertInLoopThread();
	EventLoop *nextLoop = baseLoop_;
	if(!loops_.empty())
	{
		nextLoop = loops_[next_++];
		if(static_cast<size_t>(next_) >= loops_.size())
		{
			next_ = 0;
		}
	}

	return nextLoop;
}
