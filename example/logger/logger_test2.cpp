#include <cstring>

#include <memory>

#include "lemon/logger/logger.h"

using namespace lemon::log;
int main()  {
    const char* msg = "Hello World2!\n";
    LogConfig::Ptr config = std::make_shared<LogConfig>();
    config->setLevel(LogLevel::WARN);
    config->setConsole(true);
    config->setFile("./log");

    Logger logger("user");
    logger.setConfig(config);

    logger.log(LogLevel::WARN, __FILE__, __LINE__, msg);
    logger.log(LogLevel::DEBUG, __FILE__, __LINE__, msg);

    return 0;
}