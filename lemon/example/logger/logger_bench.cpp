#include <cstring>
#include <chrono>

#include <iostream>

#include "lemon/include/logger/logger.h"

using namespace lemon::log;
using namespace std::chrono;

int main() {
    int num = 1000000;

    GLOB_CONFIG.setFile("../log/test");
    GLOB_CONFIG.setConsole(false);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < num; ++i) {
        LOG_INFO("asdasdasfadsffadgdfjjdjlaljfsdhkagnfgjaflhgjdjagnlfpufdjhfkJFKL%d\n", i);
    }

    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end - start);

    std::cout << "程序运行时间: " << duration.count() << " 毫秒" << std::endl;

    return 0;
}