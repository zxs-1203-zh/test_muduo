#ifndef CHANNEL_H
#define CHANNEL_H

#include <poll.h>
#include <functional>

namespace muduo
{

class EventLoop;

class Channel //noncopyable
{

public:
	Channel(const Channel&) = delete;
	Channel& operator=(const Channel&) = delete;

	typedef std::function<void()> EventCallback;

	Channel(EventLoop *loop, int fd);

	void handleEvent();

	void setReadCallback(const EventCallback& cb)
	{
		readEventCallback_ = cb;
	}

	void setWriteCallback(const EventCallback& cb)
	{
		writeEventCallback_ = cb;
	}

	void setErrorCallback(const EventCallback& cb)
	{
		errorEventCallback_ = cb;
	}

	int index()const
	{
		return idx_;
	}

	void set_index(int idx)
	{
		idx_ = idx;
	}

	int events()const
	{
		return events_;
	}

	void set_revents(int revents)
	{
		revents_ = revents;
	}

	bool isNoneEvent()
	{
		return events_ == kNoneEvent;
	}

	void enableReading()
	{
		events_ |= kReadEvent;
		update();
	}

	int fd()const
	{
		return fd_;
	}

	EventLoop* ownerLoop()
	{
		return loop_;
	}

private:
	void update();

	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;

	EventCallback readEventCallback_;
	EventCallback writeEventCallback_;
	EventCallback errorEventCallback_;

	EventLoop *loop_;
	int fd_;
	int events_;
	int revents_;
	int idx_;

};

}


#endif
