#include "printk.h"
#include "sbi.h"
#include "proc.h"
#include "defs.h"

extern void test();

int start_kernel()
{
    printk("[S-MODE] %d Hello RISC-V\n", 2022);

    schedule();

    csr_write(sstatus, 0x2);

    test(); // DO NOT DELETE !!!

    return 0;
}
