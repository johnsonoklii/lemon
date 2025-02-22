#include "lemon/net/buffer.h"

#include <sys/uio.h>
#include <errno.h>

using namespace lemon;
using namespace lemon::net;

ssize_t Buffer::readFd(int fd, int* saveErrno) {
    char extrabuf[65535];
    struct iovec vec[2];
    
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + m_writerIndex;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0) {
        *saveErrno = errno;
    } else if (static_cast<size_t>(n) <= writable) {
        m_writerIndex += n;
    } else {
        m_writerIndex = m_buffer.size();
        append(extrabuf, n - writable);
    }
    
    return n;
}