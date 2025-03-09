#include "lemon/base/timestamp.h"

#include <unistd.h>

using namespace lemon::base;

int main() {
    Timestamp t1 = Timestamp::now();

    sleep(3);

    Timestamp t2 = Timestamp::now();

    int64_t pad_sec = t2.secondsSinceEpoch() - t1.secondsSinceEpoch();
    int64_t pad_microsec = t2.microSecondsSinceEpoch() - t1.microSecondsSinceEpoch();
    int64_t pad_millisec = t2.milliSecondsSinceEpoch() - t1.milliSecondsSinceEpoch();

    printf("pad_sec: %ld, pad_microsec: %ld, pad_millisec: %ld\n", pad_sec, pad_microsec, pad_millisec);

    return 0;
}