#include "types.h"
#include "user/user.h"

uint64 sys_trace(void) {
    printf("hello trap\n");
    return 0;
}
uint64 sys_sysinfo(void) {
    printf("hello sysinfo\n");
    return 0;
}
