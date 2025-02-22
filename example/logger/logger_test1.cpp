#include <cstring>
#include "lemon/logger/logger.h"

using namespace lemon::log;

int main()  {
    const char* msg = "Hello World!";
    
    LOG_INFO("%s\n", msg);

    return 0;
}