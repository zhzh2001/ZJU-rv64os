#include "proc.h"
#include "mm.h"
#include "rand.h"
#include "defs.h"
#include "printk.h"
#include "panic.h"
#include "vm.h"
#include "string.h"

extern void __dummy();

struct task_struct *idle;			// idle process
struct task_struct *current;		// 指向当前运行线程的 `task_struct`
struct task_struct *task[NR_TASKS]; // 线程数组，所有的线程都保存在此

void task_init()
{
	// 1. 调用 kalloc() 为 idle 分配一个物理页
	// 2. 设置 state 为 TASK_RUNNING;
	// 3. 由于 idle 不参与调度 可以将其 counter / priority 设置为 0
	// 4. 设置 idle 的 pid 为 0
	// 5. 将 current 和 task[0] 指向 idle

	idle = (struct task_struct *)kalloc();
	idle->state = TASK_RUNNING;
	idle->counter = 0;
	idle->priority = 0;
	idle->pid = 0;
	current = idle;
	task[0] = idle;

	// 1. 参考 idle 的设置, 为 task[1] ~ task[NR_TASKS - 1] 进行初始化
	// 2. 其中每个线程的 state 为 TASK_RUNNING, counter 为 0, priority 使用 rand() 来设置, pid 为该线程在线程数组中的下标。
	// 3. 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`,
	// 4. 其中 `ra` 设置为 __dummy （见 4.3.2）的地址， `sp` 设置为 该线程申请的物理页的高地址

	extern char uapp_start[], uapp_end[];
	uint64 uapp_pgs = PGROUNDUP(uapp_end - uapp_start) / PGSIZE;
	extern unsigned long swapper_pg_dir[512];

	uint64 sstatus = csr_read(sstatus);

	for (int i = 1; i < NR_TASKS; i++)
	{
		task[i] = (struct task_struct *)kalloc();
		task[i]->state = TASK_RUNNING;
		task[i]->counter = 0;
		task[i]->priority = rand();
		task[i]->pid = i;
		task[i]->thread.ra = (uint64)__dummy;
		task[i]->thread.sp = (uint64)task[i] + PGSIZE;

		// create U-mode page table
		pagetable_t uapp_pgd = (pagetable_t)kalloc();
		memcpy(uapp_pgd, swapper_pg_dir, PGSIZE);
		uint64 uapp_pa = (uint64)alloc_pages(uapp_pgs);
		memcpy((void *)uapp_pa, uapp_start, uapp_end - uapp_start);
		memset((void *)(uapp_pa + uapp_end - uapp_start), 0, uapp_pgs * PGSIZE - (uapp_end - uapp_start));
		create_mapping(uapp_pgd, USER_START, uapp_pa - PA2VA_OFFSET, uapp_pgs, PTE_U | PTE_X | PTE_W | PTE_R | PTE_V);
		uint64 uapp_stack = (uint64)alloc_pages(1);
		create_mapping(uapp_pgd, USER_END - PGSIZE, uapp_stack - PA2VA_OFFSET, 1, PTE_U | PTE_W | PTE_R | PTE_V);

		// set U-mode trapframe
		task[i]->thread.sepc = USER_START;
		task[i]->thread.sstatus = (sstatus & ~STATUS_SPP) | STATUS_SPIE | STATUS_SUM;
		task[i]->thread.sscratch = USER_END; // user stack pointer
		task[i]->thread.pgd = (pagetable_t)((((uint64)uapp_pgd - PA2VA_OFFSET) >> 12) | 0x8000000000000000);
	}

	printk("...proc_init done!\n");
}

extern char _stext[], _srodata[];

void dummy()
{
	uint64 MOD = 1000000007;
	uint64 auto_inc_local_var = 0;
	int last_counter = -1;
	// _stext[0] = 0;
	// _srodata[0] = 0;
	// ((void (*)())(_srodata))();
	while (1)
	{
		if (last_counter == -1 || current->counter != last_counter)
		{
			last_counter = current->counter;
			auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
			printk("[PID = %ld] is running. thread space begin at 0x%lx\n", current->pid, current);
		}
	}
}

extern void __switch_to(struct thread_struct *prev, struct thread_struct *next);

void switch_to(struct task_struct *next)
{
	if (next->pid != current->pid)
	{
#ifdef SJF
		printk("\n[S-MODE] switch to [PID = %d COUNTER = %d]\n", next->pid, next->counter);
#else
		printk("\n[S-MODE] switch to [PID = %ld PRIORITY = %ld COUNTER = %ld]\n", next->pid, next->priority, next->counter);
#endif
		struct task_struct *prev = current;
		current = next;
		__switch_to(&prev->thread, &next->thread);
	}
}

void do_timer(void)
{
	/* 1. 如果当前线程是 idle 线程 或者 当前线程运行剩余时间为0 进行调度 */
	/* 2. 如果当前线程不是 idle 且 运行剩余时间不为0 则对当前线程的运行剩余时间减1 直接返回 */

	if (current->counter > 0)
		current->counter--;
	if (current->pid == 0 || current->counter == 0)
		schedule();
}

void schedule()
{
#ifdef SJF
	uint64 next = 0, c = COUNTER_MAX + 1;
	for (int i = 1; i < NR_TASKS; i++)
	{
		if (task[i]->state == TASK_RUNNING && task[i]->counter > 0 && task[i]->counter < c)
		{
			c = task[i]->counter;
			next = i;
		}
	}

	if (next)
		switch_to(task[next]);
	else
	{
		printk("\n");
		for (int i = 1; i < NR_TASKS; i++)
		{
			task[i]->counter = rand();
			printk("[S-MODE] SET [PID = %ld COUNTER = %ld]\n", task[i]->pid, task[i]->counter);
		}
		schedule();
	}
#else
#ifdef PRIORITY
	uint64 next = 0, c = 0;
	for (int i = 1; i < NR_TASKS; i++)
	{
		if (task[i]->state == TASK_RUNNING && task[i]->counter > c)
		{
			c = task[i]->counter;
			next = i;
		}
	}

	if (next)
		switch_to(task[next]);
	else
	{
		printk("\n");
		for (int i = 1; i < NR_TASKS; i++)
		{
			task[i]->counter = task[i]->priority;
			printk("[S-MODE] SET [PID = %ld PRIORITY = %ld COUNTER = %ld]\n", task[i]->pid, task[i]->priority, task[i]->counter);
		}
		schedule();
	}
#else
	panic("No scheduling algorithm selected!");
#endif
#endif
}