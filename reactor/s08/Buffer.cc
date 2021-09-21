#include "Buffer.h"

#include <sys/uio.h>

using namespace muduo;

ssize_t Buffer::readFd(int fd, int *savedErrno)
{
	char extraBuf[65536];
	struct iovec iov[2];
	iov[0].iov_base = writeBegin();
	iov[0].iov_len = buffer_.size();
	iov[1].iov_base = extraBuf;
	iov[1].iov_len = sizeof extraBuf;
	size_t writeable = writeableBytes();
	ssize_t n = ::readv(fd, iov, 2);
	if(n < 0)
	{
		*savedErrno = errno;
	}
	else if(n <= static_cast<ssize_t>(writeable))
	{
		hasWritten(n);
	}
	else
	{
		hasWritten(n);
		append(extraBuf, n - writeable);
	}
	return n;
}

void Buffer::makeSpace(size_t len)
{
	if(prependableBytes() + writeableBytes() < len + kCheapPrepend)
	{
		buffer_.resize(writeIndex_ + len);
	}
	else
	{
		size_t readable = readableBytes();
		assert(kCheapPrepend < readIndex_);
		std::copy(begin() + readIndex_,
				  begin() + writeIndex_,
				  begin() + kCheapPrepend);
		readIndex_ = kCheapPrepend;
		writeIndex_ = readIndex_ + readable;
	}
}
