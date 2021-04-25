#ifndef __HEADER_COMMON__
#define __HEADER_COMMON__

#include <chrono>

typedef unsigned int uint;

int64_t get_timestamp() {
    return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()).time_since_epoch().count();
}

int64_t get_timestamp( int64_t ts ) {
    return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()).time_since_epoch().count() - ts;
}

#endif