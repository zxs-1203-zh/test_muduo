#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include "Callbacks.h"

namespace muduo
{

class EventLoop;

class Channel//noncopyable
{

public:
	Channel(const Channel&) = delete;
	Channel& operator=(const Channel&) = delete;

	Channel(EventLoop *loop, int fd);

	int fd()const
	{
		return fd_;
	};

	int events()const
	{
		return events_;
	};

	void setRevents(int revents)
	{
		revents_ = revents;
	};

	bool isNoneEvents()const
	{
		return events_ == kNoneEvent;
	}

	int index()const
	{
		return idx_;
	}

	void setIndex(int idx)
	{
		idx_ = idx;
	}

	void setReadCallback(const EventCallback &cb)
	{
		readCallback_ = cb;
	}

	void setWriteCallback(const EventCallback &cb)
	{
		writeCallback_ = cb;
	}

	void setErrorCallback(const EventCallback &cb)
	{
		errorCallback_ = cb;
	}

	void enableReading()
	{
		events_ |= kReadEvent;
		update();
	}

	void handleEvent();

private:
	void update();

	EventLoop *loop_;
	int fd_;
	int events_;
	int revents_;
	int idx_;
	EventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback errorCallback_;

	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;
	static const int kErrorEvent;

};//Channel

}//muduo

#endif
