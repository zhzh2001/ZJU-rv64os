#include "printk.h"
#include "defs.h"

// Please do not modify

void test()
{
    for (;;)
    {
        for (int i = 0; i < 1000000000; i++)
            ;
        printk("This message is from kernel, it loops 1 billion times.\n");
    }
}
