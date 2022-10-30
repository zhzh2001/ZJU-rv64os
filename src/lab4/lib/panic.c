#include "panic.h"
#include "printk.h"

void panic(const char *msg)
{
	printk("PANIC: %s\n", msg);
	while (1)
		;
}