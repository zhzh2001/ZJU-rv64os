#ifndef _TRAP_H
#define _TRAP_H
#include "types.h"
#define TRAP_MASK 0x8000000000000000
#define TIMER_IRQ 5
#define ECALL_FROM_USER 8

struct pt_regs
{
	uint64 x[31];
	uint64 sepc;
};

void trap_handler(uint64 scause, uint64 sepc, struct pt_regs *regs);

#endif