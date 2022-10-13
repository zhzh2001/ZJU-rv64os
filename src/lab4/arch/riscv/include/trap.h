#ifndef _TRAP_H
#define _TRAP_H
#include "types.h"
#define TRAP_MASK 0x8000000000000000
#define TIMER_IRQ 5

void trap_handler(uint64 scause, uint64 sepc);

#endif