#include "lemon/base/co/fiber.h"

#include <stdio.h>
#include <unistd.h>
#include <chrono>

using namespace lemon;
using namespace lemon::co;

std::function<void()> cb;

void func() {
    printf("func start....\n");
    char buf[64];
    snprintf(buf, sizeof(buf), "hello world, %d\n", 1);
    printf("%s", buf);
    co::getThis()->yield();
    printf("func end....\n");
}

void fiber_task() {
    cb();
}

int main() {
    printf("main thread start....\n");

    co::init();

    // std::shared_ptr<Fiber> fiber(new Fiber(func));
    cb = std::bind(func);

    Fiber* fiber = new Fiber(fiber_task);

    fiber->resume();
    sleep(3);
    fiber->resume();

    printf("main thread end....\n");

    return 0;
}
