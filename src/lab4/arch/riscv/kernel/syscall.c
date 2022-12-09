#include "syscall.h"
#include "trap.h"
#include "printk.h"
#include "proc.h"
#include "mm.h"
#include "defs.h"
#include "string.h"

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
	case SYS_CLONE:
		regs->x[9] = sys_clone(regs);
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

uint64 sys_clone(struct pt_regs *regs)
{
	/*
	 1. 参考 task_init 创建一个新的 task, 将的 parent task 的整个页复制到新创建的
		task_struct 页上(这一步复制了哪些东西?）。将 thread.ra 设置为
		__ret_from_fork, 并正确设置 thread.sp
		(仔细想想，这个应该设置成什么值?可以根据 child task 的返回路径来倒推)

	 2. 利用参数 regs 来计算出 child task 的对应的 pt_regs 的地址，
		并将其中的 a0, sp, sepc 设置成正确的值(为什么还要设置 sp?)

	 3. 为 child task 申请 user stack, 并将 parent task 的 user stack
		数据复制到其中。

	 3.1. 同时将子 task 的 user stack 的地址保存在 thread_info->
		user_sp 中，如果你已经去掉了 thread_info，那么无需执行这一步

	 4. 为 child task 分配一个根页表，并仿照 setup_vm_final 来创建内核空间的映射

	 5. 根据 parent task 的页表和 vma 来分配并拷贝 child task 在用户态会用到的内存

	 6. 返回子 task 的 pid
	*/
	extern struct task_struct *current;
	struct task_struct *child_task = clone_current_task_struct();
	struct pt_regs *child_regs = (struct pt_regs *)((uint64)child_task + regs->x[1] - (uint64)current);
	extern void __ret_from_fork(void);
	child_task->thread.ra = (uint64)__ret_from_fork;
	child_task->thread.sp = (uint64)child_regs;
	child_task->thread.sscratch = csr_read(sscratch);

	child_regs->x[9] = 0; // fork return 0 in child process!
	child_regs->sepc = regs->sepc + 4;

	uint64 child_pgd = alloc_page();
	extern unsigned long swapper_pg_dir[512]; // kernel page table
	memcpy((void *)child_pgd, (void *)swapper_pg_dir, PGSIZE);
	child_task->thread.pgd = (pagetable_t)MKSATP(child_pgd);

	uint64 *parent_pgd = SATP2VA(current->thread.pgd);
	for (int i = 0; i < child_task->vma_cnt; i++)
	{
		uint64 start = child_task->vmas[i].vm_start;
		uint64 end = child_task->vmas[i].vm_end;
		for (uint64 j = start; j < end; j += PGSIZE)
		{
			uint64 vpn[3] = {VPN2(j), VPN1(j), VPN0(j)};
			uint64 *tbl = parent_pgd;
			uint64 *child_tbl = (uint64 *)child_pgd;
			for (int k = 0; k < 2; k++)
			{
				if (tbl[vpn[k]] & PTE_V)
				{
					tbl = (uint64 *)(PTE2PA(tbl[vpn[k]]) + PA2VA_OFFSET);
					if (child_tbl[vpn[k]] & PTE_V)
					{
						child_tbl = (uint64 *)(PTE2PA(child_tbl[vpn[k]]) + PA2VA_OFFSET);
					}
					else
					{
						uint64 *new_tbl = (uint64 *)alloc_page();
						memset(new_tbl, 0, PGSIZE);
						uint64 pa = (uint64)new_tbl - PA2VA_OFFSET;
						child_tbl[vpn[k]] = MKPTE(pa, PTE_V);
						child_tbl = new_tbl;
					}
				}
				else
				{
					tbl = NULL;
					break;
				}
			}
			if (tbl == NULL)
				continue;
			uint64 va = PTE2PA(tbl[vpn[2]]) + PA2VA_OFFSET;
			uint64 pa = alloc_page();
			memcpy((void *)pa, (void *)va, PGSIZE);
			child_tbl[vpn[2]] = MKPTE(pa - PA2VA_OFFSET, tbl[vpn[2]] & 0xff);
		}
	}

	return child_task->pid;
}
