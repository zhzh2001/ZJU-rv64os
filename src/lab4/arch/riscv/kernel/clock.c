#include "clock.h"
#include "sbi.h"

uint64 get_cycles()
{
	uint64 cycles;
	asm volatile("rdtime %0"
				 : "=r"(cycles));
	return cycles;
}

void clock_set_next_event()
{
	uint64 next = get_cycles() + TIMECLOCK;
	sbi_set_timer(next);
}