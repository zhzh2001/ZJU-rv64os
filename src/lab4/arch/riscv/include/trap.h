#ifndef _TRAP_H
#define _TRAP_H
#include "types.h"
#define TRAP_MASK 0x8000000000000000
#define TIMER_IRQ 5
#define ECALL_FROM_USER 8
#define INST_PAGE_FAULT 12
#define LOAD_PAGE_FAULT 13
#define STORE_PAGE_FAULT 15

struct pt_regs
{
	uint64 x[31];
	uint64 sepc;
};

void trap_handler(uint64 scause, uint64 sepc, struct pt_regs *regs);

void do_page_fault(struct pt_regs *regs, uint64 stval, uint64 scause);

#endif