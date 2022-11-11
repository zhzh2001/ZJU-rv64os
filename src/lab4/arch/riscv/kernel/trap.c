#include "trap.h"
#include "printk.h"
#include "clock.h"
#include "proc.h"

void trap_handler(uint64 scause, uint64 sepc)
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
			printk("Interrupt %ld, sepc: 0x%lx\n", scause & ~TRAP_MASK, sepc);
		}
	}
	else
	{
		// is exception
		printk("Exception: %ld, sepc: 0x%lx\n", scause, sepc);
	}
}