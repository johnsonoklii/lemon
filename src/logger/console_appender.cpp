#include "lemon/logger/console_appender.h"

#include <cstdio>

using namespace lemon;
using namespace lemon::log;

ConsoleAppender::ConsoleAppender() {
}

ConsoleAppender::~ConsoleAppender() {
}

void ConsoleAppender::append(const char* msg) {
    printf("%s", msg);
}