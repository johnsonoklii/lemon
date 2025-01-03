#include <stdio.h>

#include "lemon/include/logger/console_appender.h"

using namespace lemon;
using namespace lemon::log;

ConsoleAppender::ConsoleAppender()
{
}

ConsoleAppender::~ConsoleAppender()
{
}

void ConsoleAppender::append(const char* msg)
{
    printf("%s", msg);
}