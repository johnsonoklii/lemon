#include "lemon/net/eventloop.h"
#include "lemon/net/eventloop_threadpool.h"
#include "lemon/base/utils.h"

#include <functional>

#include "assert.h"

using namespace lemon;
using namespace lemon::net;
using namespace lemon::base;

void print(EventLoop* p = NULL) {
  printf("main(): pid = %d, tid = %d, loop = %p\n",
         getpid(), ProcessInfo::tid(), p);
}

void init(EventLoop* p) {
  printf("init(): pid = %d, tid = %d, loop = %p\n",
         getpid(), ProcessInfo::tid(), p);
}

int main() {
  print();

  EventLoop loop;
  loop.runAfter(11, std::bind(&EventLoop::stop, &loop));

  {
    printf("Single thread %p:\n", &loop);
    EventLoopThreadPool model(&loop, "single");
    model.setThreadNum(0);
    model.start(init);
    assert(model.getNextLoop() == &loop);
    assert(model.getNextLoop() == &loop);
    assert(model.getNextLoop() == &loop);
  }

  {
    printf("Another thread:\n");
    EventLoopThreadPool model(&loop, "another");
    model.setThreadNum(1);
    model.start(init);
    EventLoop* nextLoop = model.getNextLoop();
    nextLoop->runAfter(2, std::bind(print, nextLoop));
    assert(nextLoop != &loop);
    assert(nextLoop == model.getNextLoop());
    assert(nextLoop == model.getNextLoop());
    ::sleep(3);
  }

  {
    printf("Three threads:\n");
    EventLoopThreadPool model(&loop, "three");
    model.setThreadNum(3);
    model.start(init);
    EventLoop* nextLoop = model.getNextLoop();
    nextLoop->runInLoop(std::bind(print, nextLoop));
    assert(nextLoop != &loop);
    assert(nextLoop != model.getNextLoop());
    assert(nextLoop != model.getNextLoop());
    assert(nextLoop == model.getNextLoop());
  }

  loop.loop();
}