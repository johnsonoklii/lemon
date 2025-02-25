#include "lemon/base/co/fiber.h"
#include "lemon/base/logger/logger.h"

#include <memory>
#include <atomic>
#include <mutex>

#include <assert.h>

using namespace lemon;
using namespace lemon::co;
using namespace lemon::log;

// 生成协程id
static std::atomic<uint64_t> s_fiber_id{0};

// 统计当前的协程数
static std::atomic<uint64_t> s_fiber_count{0};

// 当前正在运行的协程
thread_local Fiber* t_fiber = nullptr;

// 主协程，需要使用智能指针，只会在init的时候创建，t_main_fiber来管理生命周期
thread_local Fiber::Ptr t_main_fiber = nullptr;

//内存分配器
class MallocStackAllocator
{
public:
    static void* Alloc(size_t size)
    {
        return malloc(size);
    }

    static void Dealloc(void* vp)
    {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

thread_local std::once_flag inited;
void lemon::co::init() {
    std::call_once(inited, []() {
        Fiber::Ptr main_fiber = std::make_shared<Fiber>();
        assert(t_fiber == main_fiber.get());
        t_main_fiber = main_fiber;
    });
}

Fiber* lemon::co::getThis() {
    return t_fiber;
}

void lemon::co::setThis(Fiber* fiber) {
    t_fiber = fiber;
}

Fiber::Fiber() {
    setThis(this);
    setState(RUNING);

    if (getcontext(&m_ctx)) {
        LOG_FATAL("Fiber::Fiber(): getcontext\n");
    }

    ++s_fiber_count;
    m_id = s_fiber_id++;

    LOG_DEBUG("Fiber::Fiber(): main fiber id = %ld\n", m_id);
}

Fiber::Fiber(const FunCallback& cb, size_t statck_size)
: m_cb(cb), m_stack_size(statck_size) {
    ++s_fiber_count;
    m_id = s_fiber_id++;

    setState(READY);
    m_stack = StackAllocator::Alloc(m_stack_size);

    if (getcontext(&m_ctx)) {
        LOG_FATAL("Fiber::Fiber(): getcontext\n");
    }

    m_ctx.uc_link          = nullptr;
    m_ctx.uc_stack.ss_sp   = m_stack;
    m_ctx.uc_stack.ss_size = m_stack_size;
 
    makecontext(&m_ctx, &Fiber::run, 0);

    LOG_DEBUG("Fiber::Fiber(): sub fiber id = %ld\n", m_id);
}

Fiber::~Fiber() {
    if (this == t_main_fiber.get()) {
        // 主协程
        assert(m_state = RUNING);
        if (this == t_fiber) {
            setThis(nullptr);
        }
        LOG_DEBUG("Fiber::~Fiber(): main fiber id = %ld\n", m_id);
    } else {
        // 子协程
        assert(m_state != RUNING);
        StackAllocator::Dealloc(m_stack);
        LOG_DEBUG("Fiber::~Fiber(): sub fiber id = %ld\n", m_id);
    }

    --s_fiber_count;
}

void Fiber::resume() {
    assert(m_state != TERM);
    setThis(this);
    setState(RUNING);
    
    if (swapcontext(&t_main_fiber->m_ctx, &m_ctx)) {
        LOG_FATAL("Fiber::resume(): swapcontext\n");
    }
}

void Fiber::yield() {
    setThis(t_main_fiber.get());

    if (m_state == RUNING) {
        setState(HOLD);
    }
    
    if (swapcontext(&m_ctx, &t_main_fiber->m_ctx)) {
        LOG_FATAL("Fiber::yield(): swapcontext\n");
    }
}

void Fiber::reset(const FunCallback& cb) {
    assert(m_state == READY || m_state == TERM);
    m_cb = cb;
    setState(READY);
    
    if (getcontext(&m_ctx)) {
        LOG_FATAL("Fiber::reset: getcontext\n");
    }

    m_ctx.uc_link          = nullptr;
    m_ctx.uc_stack.ss_sp   = m_stack;
    m_ctx.uc_stack.ss_size = m_stack_size;
 
    makecontext(&m_ctx, &Fiber::run, 0);
}

void Fiber::run() {
    Fiber* cur = getThis();
    assert(cur);
    assert(cur->getState() != TERM);

    cur->m_cb();
    cur->m_cb = nullptr;
    cur->setState(TERM);
    
    cur->yield(); // 协程结束时自动yield，以回到主协程
}


