#include <cstring>

#include <memory>

#include "lemon/logger/logger.h"
#include "lemon/base/utils.h"

using namespace lemon::log;
using namespace lemon::base;

int main()  {
    const char* msg = "Hello World2!\n";
    LogConfig::Ptr config = std::make_shared<LogConfig>();
    config->setLevel(LogLevel::WARN);
    config->setConsole(true);
    config->setFile("./log");

    Logger logger("user");
    logger.setConfig(config);

    logger.log(LogLevel::WARN, __FILE__, __LINE__, ProcessInfo::tid(), msg);
    logger.log(LogLevel::DEBUG, __FILE__, __LINE__, ProcessInfo::tid(), msg);

    return 0;
}