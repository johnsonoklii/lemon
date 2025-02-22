#ifndef LEMON_BUFFER_H
#define LEMON_BUFFER_H

#include <vector>
#include <algorithm>
#include <string>

#include <string.h>
#include <stdint.h>
#include <cassert>

namespace lemon {
namespace net {
/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
    : m_buffer(kCheapPrepend + initialSize)
     , m_readerIndex(kCheapPrepend)
     , m_writerIndex(kCheapPrepend) {
    
    }

    void swap(Buffer& rhs) {
        if (this == &rhs) return;

        m_buffer.swap(rhs.m_buffer);
        std::swap(m_readerIndex, rhs.m_readerIndex);
        std::swap(m_writerIndex, rhs.m_writerIndex);
    }

    size_t readableBytes() const {
        return m_writerIndex - m_readerIndex;
    }

    size_t writableBytes() const {
        return m_buffer.size() - m_writerIndex;
    }

    size_t prependableBytes() const {
        return m_readerIndex;
    }

    const char* peek() const {
        return begin() + m_readerIndex;
    }

    void retrieve(size_t len) {
        assert(len <= readableBytes());
        if (len < readableBytes()) {
            m_readerIndex += len;
        } else {
            retrieveAll();
        }
    }

    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len)
    {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    void retrieveInt64() {
        retrieve(sizeof(int64_t));
    }

    void retrieveInt32() {
        retrieve(sizeof(int32_t));
    }

    void retrieveInt16() {
        retrieve(sizeof(int16_t));
    }

    void retrieveInt8() {
        retrieve(sizeof(int8_t));
    }

    void retrieveAll() {
        m_readerIndex = kCheapPrepend;
        m_writerIndex = kCheapPrepend;
    }

    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        hasWritten(len);
    }

    char* beginWrite() {
        return begin() + m_writerIndex;
    }

    const char* beginWrite() const {
        return begin() + m_writerIndex;
    }

    void hasWritten(size_t len) {
        assert(len <= writableBytes());
        m_writerIndex += len;
    }

    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }

        assert(writableBytes() >= len);
    }

    ssize_t readFd(int fd, int* saveErrno);

private:

    char* begin() {
        return m_buffer.data();
    }

    const char* begin() const {
        return m_buffer.data();
    }

    void makeSpace(size_t len) {
        if (len - writableBytes() > prependableBytes() - kCheapPrepend) {
            // writeable + 已经读了的区域  < len
            m_buffer.resize(m_writerIndex + len);
        } else {
            // 移动已经读了的区域
            assert(kCheapPrepend < m_readerIndex);
            size_t readale = readableBytes();
            std::copy(begin()+m_readerIndex,
                      begin()+m_writerIndex,
                      begin()+kCheapPrepend);

            m_readerIndex = kCheapPrepend;
            m_writerIndex = m_readerIndex + readale;
            assert(readale == readableBytes());
        }
    }

    std::vector<char> m_buffer;
    size_t m_readerIndex;
    size_t m_writerIndex;

    static const char kCRLF[];
};

} // namespace net
} // namespace lemon

#endif