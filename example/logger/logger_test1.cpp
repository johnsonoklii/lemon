#include <cstring>

#include "lemon/logger/logger.h"

using namespace lemon::log;
int main()  {
    const char* msg = "Hello World!";
    
    // GLOB_CONFIG.setFile("log.txt");
    LOG_INFO("%s\n", msg);

    return 0;
}