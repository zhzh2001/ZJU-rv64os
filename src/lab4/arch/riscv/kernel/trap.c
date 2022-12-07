#include "trap.h"
#include "printk.h"
#include "clock.h"
#include "proc.h"
#include "syscall.h"

void trap_handler(uint64 scause, uint64 sepc, struct pt_regs *regs)
{
	if (scause & TRAP_MASK)
	{
		// is interrupt
		if ((scause & ~TRAP_MASK) == TIMER_IRQ)
		{
			// printk("Timer interrupt: %ld\n", get_cycles());
			clock_set_next_event();
			do_timer();
		}
		else
		{
			printk("[S-MODE] Interrupt %ld, sepc: 0x%lx\n", scause & ~TRAP_MASK, sepc);
		}
	}
	else
	{
		if (scause == ECALL_FROM_USER)
		{
			syscall_handler(regs);
			regs->sepc += 4; // skip ecall instruction
		}
		else
		{
			printk("[S-MODE] Exception: %ld, sepc: 0x%lx\n", scause, sepc);
		}
	}
}