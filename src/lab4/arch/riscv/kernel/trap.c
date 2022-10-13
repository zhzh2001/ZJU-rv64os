#include "trap.h"
#include "printk.h"
#include "clock.h"

void trap_handler(uint64 scause, uint64 sepc)
{
	if (scause & TRAP_MASK)
	{
		// is interrupt
		if ((scause & ~TRAP_MASK) == TIMER_IRQ)
		{
			printk("Timer interrupt: %ld\n", get_cycles());
			clock_set_next_event();
		}
		else
		{
			printk("Interrupt %d, sepc: 0x%x\n", scause & ~TRAP_MASK, sepc);
		}
	}
	else
	{
		// is exception
		printk("Exception: %d, sepc: 0x%x\n", scause, sepc);
	}
}