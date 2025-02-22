#include "lemon/base/count_down_latch.h"

using namespace lemon;
using namespace lemon::base;

CountDownLatch::CountDownLatch(int count)
    : m_count(count), m_mutex(), m_cv() {}

void CountDownLatch::wait() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_count > 0) {
        m_cv.wait(lock);
    }
}

int32_t CountDownLatch::getCount() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_count;
}

void CountDownLatch::countDown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_count--;
    if (m_count == 0) {
      m_cv.notify_one();
    }
}

