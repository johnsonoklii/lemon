#include "lemon/base/co/fiber.h"

#include <stdio.h>
#include <unistd.h>
#include <chrono>

using namespace lemon;
using namespace lemon::co;

void func() {
    printf("func start....\n");
    co::getThis()->yield();
    printf("func end....\n");
}

int main() {
    printf("main thread start....\n");

    co::init();

    // std::shared_ptr<Fiber> fiber(new Fiber(func));
    Fiber* fiber = new Fiber(func);

    fiber->resume();
    sleep(3);
    fiber->resume();

    printf("main thread end....\n");

    return 0;
}
