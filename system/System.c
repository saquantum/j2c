#include "System.h"

void System$exit$0(int e){
    exit(e);
}

long System$currentTimeMillis$0(){
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec*1000 + ts.tv_nsec/1000000;
}
