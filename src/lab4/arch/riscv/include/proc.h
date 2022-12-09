#ifndef _PROC_H
#define _PROC_H
#include "types.h"

#define NR_TASKS (1 + 16) // 用于控制 最大线程数量 （idle 线程 + 1 内核线程）
#define INIT_TASK (1 + 1) // 用于控制 内核线程的起始编号

#define TASK_RUNNING 0 // 为了简化实验，所有的线程都只有一种状态

#define PRIORITY_MIN 1
#define PRIORITY_MAX 10
#define COUNTER_MAX 10

/* 用于记录 `线程` 的 `内核栈与用户栈指针` */
/* (lab6中无需考虑，在这里引入是为了之后实验的使用) */
// deprecated!
/*
struct thread_info
{
	uint64 kernel_sp;
	uint64 user_sp;
};
*/

typedef unsigned long *pagetable_t;

/* 线程状态段数据结构 */
struct thread_struct
{
	uint64 ra;
	uint64 sp;
	uint64 s[12];

	uint64 sepc, sstatus, sscratch;

	pagetable_t pgd;
};

#define VM_X_MASK 0x0000000000000008
#define VM_W_MASK 0x0000000000000004
#define VM_R_MASK 0x0000000000000002
#define VM_ANONYM 0x0000000000000001

struct vm_area_struct
{
	uint64 vm_start; /* VMA 对应的用户态虚拟地址的开始   */
	uint64 vm_end;	 /* VMA 对应的用户态虚拟地址的结束   */
	uint64 vm_flags; /* VMA 对应的 flags */
};

/* 线程数据结构 */
struct task_struct
{
	// struct thread_info *thread_info;
	uint64 state;	 // 线程状态
	uint64 counter;	 // 运行剩余时间
	uint64 priority; // 运行优先级 1最低 10最高
	uint64 pid;		 // 线程id

	struct thread_struct thread;

	uint64 vma_cnt;				   /* 下面这个数组里的元素的数量 */
	struct vm_area_struct vmas[0]; /* 为什么可以开大小为 0 的数组?
					   这个定义可以和前面的 vma_cnt 换个位置吗? */
};

/* 线程初始化 创建 NR_TASKS 个线程 */
void task_init();

/* 在时钟中断处理中被调用 用于判断是否需要进行调度 */
void do_timer();

/* 调度程序 选择出下一个运行的线程 */
void schedule();

/* 线程切换入口函数*/
void switch_to(struct task_struct *next);

/* dummy funciton: 一个循环程序，循环输出自己的 pid 以及一个自增的局部变量*/
void dummy();

void do_mmap(struct task_struct *task, uint64 addr, uint64 length, uint64 flags);

struct vm_area_struct *find_vma(struct task_struct *task, uint64 addr);

struct task_struct *clone_current_task_struct();

#endif