#ifndef _SYSCALL_H
#define _SYSCALL_H
#include "trap.h"
#include "types.h"

#define SYS_WRITE 64
#define SYS_GETPID 172
#define SYS_CLONE 220

void syscall_handler(struct pt_regs *regs);
uint64 sys_write(unsigned int fd, const char *buf, uint64 count);
uint64 sys_getpid(void);
uint64 sys_clone(struct pt_regs *regs);

#endif