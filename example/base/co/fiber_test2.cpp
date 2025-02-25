#include "lemon/base/co/fiber.h"

#include <stdio.h>
#include <unistd.h>
#include <chrono>

using namespace lemon;
using namespace lemon::co;

void func1() {
    printf("func start....\n");
    co::getThis()->yield();
    printf("func end....\n");
}

void func2() {
    printf("func2 start....\n");
    co::getThis()->yield();
    printf("func2 end....\n");
}

int main() {
    printf("main thread start....\n");

    co::init();

    // std::shared_ptr<Fiber> fiber(new Fiber(func));
    Fiber* fiber = new Fiber(func1);

    fiber->resume();
    sleep(2);
    fiber->resume();

    fiber->reset(func2);
    fiber->resume();
    sleep(2);
    fiber->resume();

    printf("main thread end....\n");

    return 0;
}
