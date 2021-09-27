#pragma once

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
#include <string>

namespace muduo
{

class Buffer//copyable
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

	//default ctor and copy are ok
	
	void swap(Buffer &rhs)
	{
		buffer_.swap(rhs.buffer_);
		std::swap(readIndex_, rhs.readIndex_);
		std::swap(writeIndex_, rhs.readIndex_);
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

	const char *peek()const
	{
		return begin() + readIndex_;
	}

	void retrieve(size_t len)
	{
		assert(len <= readableBytes());
		readIndex_ += len;
	}

	void retrieveUntil(const char *end)
	{
		assert(peek() <= end);
		assert(end <= beginWrite());
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

	void append(const char *data, size_t len)
	{
		ensureWriteableBytes(len);
		std::copy(data, data + len, beginWrite());
		hasWriten(len);
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
		if(writeableBytes() < len)
		{
			makeSpace(len);
		}
		assert(writeableBytes() >= len);
	}

	char *beginWrite()
	{
		return begin() + writeIndex_;
	}

	const char *beginWrite()const
	{
		return begin() + writeIndex_;
	}

	void hasWriten(size_t len)
	{
		writeIndex_ += len;
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
