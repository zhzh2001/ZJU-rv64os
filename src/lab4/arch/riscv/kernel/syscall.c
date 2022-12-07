#include "syscall.h"
#include "trap.h"
#include "printk.h"
#include "proc.h"

void syscall_handler(struct pt_regs *regs)
{
	switch (regs->x[16])
	{
	case SYS_WRITE:
		regs->x[9] = sys_write(regs->x[9], (const char *)regs->x[10], regs->x[11]);
		break;
	case SYS_GETPID:
		regs->x[9] = sys_getpid();
		break;
	default:
		printk("Unknown syscall: %ld\n", regs->x[16]);
		break;
	}
}

uint64 sys_write(unsigned int fd, const char *buf, uint64 count)
{
	if (fd == 1 || fd == 2)
	{
		for (int i = 0; i < count; i++)
		{
			printk("%c", buf[i]);
		}
		return count;
	}
	else
	{
		return -1;
	}
}

uint64 sys_getpid(void)
{
	extern struct task_struct *current;
	return current->pid;
}