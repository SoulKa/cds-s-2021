#ifndef __HEADER_COMMON__
#define __HEADER_COMMON__

#include <time.h>

typedef unsigned int uint;

clock_t get_timestamp() {
    return clock();
}

clock_t get_timediff( clock_t ts ) {
    return clock() - ts;
}

#endif