#pragma once

#include <algorithm>
#include <cassert>
#include <vector>
#include <string>

namespace muduo
{

class Buffer
{

public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;

	Buffer():
		buffer_(kCheapPrepend + kInitialSize),
		readIndex_(kCheapPrepend),
		writeIndex_(kCheapPrepend)
	{
		assert(readableBytes() == 0);
		assert(writeableBytes() == kInitialSize);
		assert(prependableBytes() == kCheapPrepend);
	}

	size_t readableBytes()
	{
		return writeIndex_ - readIndex_;
	}

	size_t writeableBytes()
	{
		return buffer_.size() - writeIndex_;
	}

	size_t prependableBytes()
	{
		return readIndex_;
	}

	const char *peek()
	{
		return begin() + readIndex_;
	}

	void retrieve(size_t len)
	{
		assert(len < readableBytes());
		readIndex_ += len;
	}

	void retrieveUntil(const char *end)
	{
		assert(end > peek());
		assert(end <= writeBegin());
		retrieve(end - peek());
	}

	void retrieveAll()
	{
		readIndex_ = kCheapPrepend;
		writeIndex_ = kCheapPrepend;
	}

	std::string retrieveAsString()
	{
		std::string str(peek(), writeableBytes());
		retrieveAll();
		return str;
	}

	char *writeBegin()
	{
		return begin() + writeIndex_;
	}

	const char *writeBegin()const
	{
		return begin() + writeIndex_;
	}

	void hasWritten(size_t len)
	{
		assert(len < writeableBytes());
		writeIndex_ += len;
	}

	void append(const char *data, size_t len)
	{
		ensureWriteableBytes(len);
		assert(writeableBytes() >= len);
		std::copy(data, data + len, writeBegin());
	}

	void append(const void *data, size_t len)
	{
		append(static_cast<const char*>(data), len);
	}

	void append(const std::string &str)
	{
		append(str.data(), str.size());
	}

	void ensureWriteableBytes(size_t len)
	{
		if(len > writeableBytes())
		{
			makeSpace(len);
		}
		assert(writeableBytes() > len);
	}

	ssize_t readFd(int fd, int *savedErrno);

private:
	char *begin()
	{
		return buffer_.data();
	}

	const char *begin()const
	{
		return buffer_.data();
	}

	void makeSpace(size_t len);

	std::vector<char> buffer_;
	size_t readIndex_;
	size_t writeIndex_;

};//Buffer

}//muduo
