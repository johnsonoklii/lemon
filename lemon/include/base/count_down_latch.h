#ifndef COUNT_DOWN_LATH_H_
#define COUNT_DOWN_LATH_H_


#include <mutex>
#include <condition_variable>

namespace lemon {
namespace base {

class CountDownLatch {
public:
  explicit CountDownLatch(int count);
  void wait();
  void countDown();
  int32_t getCount();

private:
  int32_t m_count = 0;
  std::mutex m_mutex;
  std::condition_variable m_cv;
};

} // namespace base
} // namespace lemon

#endif