#ifndef LEMON_LOG_BUFFER_H
#define LEMON_LOG_BUFFER_H

#include <cstddef>
#include <string>

#include "lemon/base/nocopyable.h"

namespace lemon {
namespace log {

enum { kSmallBuffer = 4096, kLargeBuffer = 65536 };

struct inner_message {
    // FIXME: 为什么const char* 会出现乱码?
    // const char* msg; 
    std::string msg;
};

template <int SIZE>
class LogBuffer: public base::noncopyable {
public:
    LogBuffer() : m_data(new inner_message[SIZE]), m_current(m_data){}
    
    ~LogBuffer() { delete[] m_data; }

    LogBuffer(LogBuffer&& other) noexcept
    : m_data(other.m_data), m_current(other.m_current) {
        other.m_data = nullptr;
        other.m_current = nullptr;
    }

    LogBuffer& operator=(LogBuffer&& other) noexcept {
        m_data = other.m_data;
        m_current = other.m_current;
        other.m_data = nullptr;
        other.m_current = nullptr;
        return *this;
    }

    bool valid() const { return m_data != nullptr; }

    int avail() { return static_cast<int>(end_addr() - m_current); }
    
    void push(const inner_message& msg) {
        if (avail() > 0) {
            *m_current = msg;
            m_current++;
        }
    }

    inner_message* begin() const { return m_data; }
    inner_message* end() const { return m_current; }

    void reset() { m_current = m_data; }

private:
    inner_message* end_addr() { return m_data + SIZE; }

    inner_message* m_data;
    inner_message* m_current;
};

} // namespace log    
} // namespace lemon

#endif