#ifndef LEMON_FIBER_H
#define LEMON_FIBER_H

#include "lemon/base/nocopyable.h"

#include <functional>
#include <memory>

#include <stdint.h>
#include <ucontext.h>

namespace lemon {
namespace co {
using namespace lemon::base;

constexpr int STACK_SIZE = 128 * 1024; // 128KB

// only run in thread who created it
class Fiber: noncopyable, public std::enable_shared_from_this<Fiber> {
public:
    enum State {
        READY = 0,
        HOLD,
        RUNING,
        TERM,
    };

    using FunCallback = std::function<void()>;
    using Ptr = std::shared_ptr<Fiber>;

    Fiber();
    Fiber(const FunCallback& cb, size_t statck_size = STACK_SIZE);
    ~Fiber();

    void resume();
    void yield();
    void reset(const FunCallback& cb);

    State getState() const { return m_state; }
    void setState(State state) { m_state = state; }
    uint64_t getId() const { return m_id; }

    static Fiber* createMainFiber();

private:
    static void run();
    
    FunCallback m_cb;
    uint64_t m_id;
    State m_state;
    ucontext_t m_ctx;
    size_t m_stack_size;
    void* m_stack;
    
};

// 创建一个主协程，必须在使用fiber之前调用
void init();

// 在协程函数中，只能使用Fiber*，不要使用智能指针
// 如果使用智能指针，在协程函数执行完后，会执行析构，释放m_stack。
// 但此时协程还在运行，会使用到m_stack
Fiber* getThis();
void setThis(Fiber* fiber);

} // namespace co
} // namespace lemon

#endif