#include "printk.h"
#include "sbi.h"

extern void test();

int start_kernel()
{
    printk("%d Hello RISC-V\n", 2022);

    // puts("Shutting down...\n");
    // sbi_shutdown();

    test(); // DO NOT DELETE !!!

    return 0;
}
