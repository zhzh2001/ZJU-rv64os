#include "print.h"
#include "sbi.h"

extern void test();

int start_kernel() {
    puti(2022);
    puts(" Hello RISC-V\n");

    puts("Shutting down...\n");
    sbi_shutdown();

    test(); // DO NOT DELETE !!!

	return 0;
}
