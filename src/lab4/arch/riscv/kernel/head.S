.extern start_kernel

    .section .text.init
    .globl _start
_start:
    # setup stack
    la sp, boot_stack_top

    # setup vm
    call setup_vm
    call relocate

    # setup memory management
    call mm_init

    # setup vm final
    call setup_vm_final

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

relocate:
    # set ra = ra + PA2VA_OFFSET
    # set sp = sp + PA2VA_OFFSET (If you have set the sp before)

    li t0, 0xffffffe000000000-0x0000000080000000
    add ra, ra, t0
    add sp, sp, t0

    # set satp with early_pgtbl

    la t0, early_pgtbl
    srli t0, t0, 12           # ppn = pa >> 12
    li t1, 0x8000000000000000 # mode = sv39
    or t0, t0, t1
    csrw satp, t0

    # flush tlb
    sfence.vma zero, zero

    ret

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096

    .globl boot_stack_top
boot_stack_top: