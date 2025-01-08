#include <stdio.h>

#include <vector>

#include "lemon/logger/log_buffer.h"

using namespace lemon;
using namespace lemon::log;

int main() {
    int num = 100;

    LogBuffer<1024> buffer;
    
    for (int i = 0; i < num; ++i) {
        buffer.push({"asdasdasfadsffadgdfjjdjlaljfsdhkagnfgjaflhgjdjagnlfpufdjhfkJFKL\n"});
    }
    
    std::vector<LogBuffer<1024>> buffers;

    buffers.push_back(std::move(buffer));

    std::vector<LogBuffer<1024>> buffer_write;
    buffer_write.reserve(16);

    buffer_write.swap(buffers);

    for (const auto& buffer : buffer_write) {
        for (const auto& it : buffer) {
            printf("%s", it.msg.c_str());
        }
    }
}