#include "time.h"
#include "share.h"
#include "jl17467pqw.h"

void time_init()
{
    local_time.second = 0;
    local_time.minute = 0;
    local_time.hour = 0;
}


void time_sync()
{
    jt17467pqw_set_time(local_time);
}