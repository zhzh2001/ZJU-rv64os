#include "trap.h"
#include "printk.h"
#include "clock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"
#include "vm.h"
#include "string.h"
#include "mm.h"

void trap_handler(uint64 scause, uint64 sepc, struct pt_regs *regs)
{
	if (scause & TRAP_MASK)
	{
		// is interrupt
		if ((scause & ~TRAP_MASK) == TIMER_IRQ)
		{
			clock_set_next_event();
			do_timer();
		}
		else
		{
			printk("[S-MODE] Unhandled Interrupt %ld, sepc: 0x%lx\n", scause & ~TRAP_MASK, sepc);
			for (;;)
				;
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
			uint64 stval = csr_read(stval);
			if (scause == INST_PAGE_FAULT || scause == LOAD_PAGE_FAULT || scause == STORE_PAGE_FAULT)
				do_page_fault(regs, stval, scause);
			else
			{
				printk("[S-MODE] Unhandled Exception: %ld, sepc: 0x%lx, stval: 0x%lx\n", scause, sepc, stval);
				for (;;)
					;
			}
		}
	}
}

void do_page_fault(struct pt_regs *regs, uint64 stval, uint64 scause)
{
	/*
	 1. 通过 stval 获得访问出错的虚拟内存地址（Bad Address）
	 2. 通过 find_vma() 查找 Bad Address 是否在某个 vma 中
	 3. 分配一个页，将这个页映射到对应的用户地址空间
	 4. 通过 (vma->vm_flags | VM_ANONYM) 获得当前的 VMA 是否是匿名空间
	 5. 根据 VMA 匿名与否决定将新的页清零或是拷贝 uapp 中的内容
	*/
	uint64 bad_addr = stval;
	printk("Page fault: 0x%lx, sepc: 0x%lx, scause: %ld\n", bad_addr, regs->sepc, scause);
	extern struct task_struct *current;
	struct vm_area_struct *vma = find_vma(current, bad_addr);
	if (vma == NULL)
	{
		printk("Address not in vma: 0x%lx, sepc: 0x%lx, scause: %ld\n", bad_addr, regs->sepc, scause);
		for (;;)
			;
	}
	if (scause == INST_PAGE_FAULT && !(vma->vm_flags & VM_X_MASK))
	{
		printk("Address not executable: 0x%lx, sepc: 0x%lx, scause: %ld\n", bad_addr, regs->sepc, scause);
		for (;;)
			;
	}
	if (scause == LOAD_PAGE_FAULT && !(vma->vm_flags & VM_R_MASK))
	{
		printk("Address not readable: 0x%lx, sepc: 0x%lx, scause: %ld\n", bad_addr, regs->sepc, scause);
		for (;;)
			;
	}
	if (scause == STORE_PAGE_FAULT && !(vma->vm_flags & VM_W_MASK))
	{
		printk("Address not writable: 0x%lx, sepc: 0x%lx, scause: %ld\n", bad_addr, regs->sepc, scause);
		for (;;)
			;
	}
	uint64 va = alloc_page();
	if (va == 0)
	{
		printk("Out of memory, sepc: 0x%lx, scause: %ld\n", regs->sepc, scause);
		for (;;)
			;
	}
	uint64 bad_addr_aligned = PGROUNDDOWN(bad_addr);
	int perm = PTE_U | PTE_V;
	if (vma->vm_flags & VM_R_MASK)
		perm |= PTE_R;
	if (vma->vm_flags & VM_W_MASK)
		perm |= PTE_W;
	if (vma->vm_flags & VM_X_MASK)
		perm |= PTE_X;
	create_mapping(SATP2VA(current->thread.pgd), bad_addr_aligned, va - PA2VA_OFFSET, PGSIZE, perm);
	extern char uapp_start[];
	if (vma->vm_flags & VM_ANONYM)
		memset((void *)va, 0, PGSIZE);
	else
		memcpy((void *)va, (void *)(uapp_start + (bad_addr_aligned - vma->vm_start)), PGSIZE);
}
