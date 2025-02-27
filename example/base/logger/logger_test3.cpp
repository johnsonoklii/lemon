#include <cstring>

#include <iostream>

#include "lemon/base/logger/logger.h"

using namespace lemon::log;
int main()  {
    try {
        const char* msg = "Hello World3!";
        
        GLOB_LOG_CONFIG.setFile("log3.txt");
        GLOB_LOG_CONFIG.setAsync(true);
        LOG_INFO("%s\n", msg);
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    
    return 0;
}