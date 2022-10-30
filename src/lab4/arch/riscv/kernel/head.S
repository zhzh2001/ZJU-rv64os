.extern start_kernel

    .section .text.init
    .globl _start
_start:
    # setup stack
    la sp, boot_stack_top

    # setup memory management
    call mm_init

    # init tasks
    call task_init

    # setup trap vector
    la t0, _traps
    csrw stvec, t0

    # enable timer interrupt
    # `csrsi sie, 0x20` is invalid since imm only has 5 bits
    li t0, 0x20
    csrs sie, t0

    # set first timer interrupt
    rdtime t0
    li t1, 10000000
    add a0, t0, t1
    li a6, 0 # fid
    li a7, 0 # eid = timer interrupt
    ecall

    # enable global interrupt
    csrsi sstatus, 0x2

    j start_kernel

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096

    .globl boot_stack_top
boot_stack_top: