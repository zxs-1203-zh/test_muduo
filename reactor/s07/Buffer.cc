#include <algorithm>
#include <sys/uio.h>

#include "Buffer.h"

using namespace muduo;

ssize_t Buffer::readFd(int fd, int *savedErrno)
{
	char extraBuf[65536];
	struct iovec iov[2]; 
	iov[0].iov_base = beginWrite();
	iov[0].iov_len = writeableBytes();
	iov[1].iov_base = extraBuf;
	iov[1].iov_len = sizeof extraBuf;
	const ssize_t n = ::readv(fd, iov, 2);
	if(n < 0)
	{
		*savedErrno = errno;
	}
	else if(static_cast<size_t>(n) < writeableBytes())
	{
		hasWriten(n);
	}
	else
	{
		hasWriten(writeableBytes());
		append(extraBuf, n - writeableBytes());
	}
	return n;
}

void Buffer::makeSpace(size_t len)
{
	if(writeableBytes() + prependableBytes() - kCheapPrepend <
	   len)
	{
		buffer_.resize(writeIndex_ + len);
	}
	else
	{
		assert(kCheapPrepend < readIndex_); 
		size_t readable = readableBytes();
		std::copy(begin() + readIndex_, 
				  begin() + writeIndex_, 
				  begin() + kCheapPrepend);
		readIndex_ = kCheapPrepend;
		writeIndex_ = readIndex_ + readable;
		assert(readable == readableBytes());
	}
}
