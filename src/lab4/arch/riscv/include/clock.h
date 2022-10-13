#ifndef _CLOCK_H
#define _CLOCK_H
#include "types.h"
#define TIMECLOCK 10000000

uint64 get_cycles();
void clock_set_next_event();

#endif