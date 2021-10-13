test1:
    lui x1, 0xDEADB     # 0xDEADB000
    ori x2, x0, 0xEF    # 0x000000EF
    addi x3, x0, 1      # 0x00000001
    nop
    nop
    add x4, x1, x2      # 0xDEADB0EF
    sub x5, x2, x1      # 0x215250EF
    nop
    nop
    srl x6, x4, x3      # 0x6F56D877
    and x7, x5, x6      # 0x21525067
    lui x8, 0x21525     # 0x21525000
    nop
    nop
    nop
    addi x9, x8, 0x67   # 0x21525067
    nop
    nop
    nop
    bne x7, x9, fail
    nop
    nop
    nop
    nop
    nop

test2:
    sw x4, 0(x0)        # mem[0] = 0xDEADB0EF
    lb x1, 1(x0)        # 0xFFFFFFB0
    lbu x2, 2(x0)       # 0x000000AD
    sb x3, 3(x0)        # mem[0] = 0x01ADB0EF
    lw x4, 0(x0)        # 0x01ADB0EF
    lui x5, 0x1ADB      # 0x01ADB000
    srli x6, x4, 12     # 0x00001ADB
    nop
    nop
    nop
    slli x7, x6, 12     # 0xADADB000
    nop
    nop
    nop
    bne x5, x7, fail
    nop
    nop
    nop
    nop
    nop

test3:
    addi x1, x0, 0xf4   # 0x000000f4
    nop
    nop
    nop
    jalr x1, x1, 0      # 0x000000c8
    nop
    nop
    nop
    nop
    nop

fail:
    j fail              # 20th ins from 0
    nop
    nop
    nop
    nop
    nop
    addi x2, x0, 0xc8   # 0x000000c8
    nop
    nop
    nop
    bne x1, x2, fail
    nop
    nop
    nop
    nop
    nop

pass:
    j pass
    nop
    nop
    nop
    nop
    nop
