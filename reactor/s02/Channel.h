#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>

#include <muduo/base/Logging.h>

namespace muduo
{

class EventLoop;

class Channel //noncopyable
{

public:
	Channel(const Channel&) = delete;
	Channel& operator=(const Channel&) = delete;

	Channel(EventLoop *loop, int fd);
	~Channel();

	typedef std::function<void()> EventCallback;

	void setReadCallback(const EventCallback& cb)
	{
		readCallback_ = cb;
	}

	void setWriteCallback(const EventCallback& cb)
	{
		writeCallback_ = cb;
	}

	void setErrorCallback(const EventCallback& cb)
	{
		errorCallback_ = cb;
	}

	void enableReading()
	{
		events_ |= kReadEvent;
		update();
	};

	int fd()
	{
		return fd_;
	}

	int event()
	{
		return events_;
	}

	void setRevent(int revent)
	{
		revents_ = revent;
	}

	bool isNonEvent()
	{
		return events_ == 0;
	}

	int index()
	{
		return idx_;
	}

	void setIndex(int idx)
	{
		idx_ = idx;
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

	static const int kReadEvent;
	static const int kWriteEvent;
	static const int kErrorEvent;

};//Channel

};//muduo

#endif
