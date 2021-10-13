test1:
    lui x1, 0xDEADB     # 0xDEADB000
    ori x2, x0, 0xEF    # 0x000000EF
    addi x3, x0, 1      # 0x00000001
    add x4, x1, x2      # 0xDEADB0EF
    sub x5, x2, x1      # 0x215250EF
    srl x6, x4, x3      # 0x6F56D877
    and x7, x5, x6      # 0x21525067
    lui x8, 0x21525     # 0x21525000
    nop
    addi x9, x8, 0x67   # 0x21525067
    bne x7, x9, fail

test2:
    sw x4, 0(x0)        # mem[0] = 0xDEADB0EF
    lb x1, 1(x0)        # 0xFFFFFFB0
    lbu x2, 2(x0)       # 0x000000AD
    sb x3, 3(x0)        # mem[0] = 0x01ADB0EF
    lw x4, 0(x0)        # 0x01ADB0EF
    lui x5, 0x1ADB      # 0x01ADB000
    srli x6, x4, 12     # 0x00001ADB
    nop
    slli x7, x6, 12     # 0xADADB000
    nop
    bne x5, x7, fail

test3:
    addi ra, x0, 0x64   # 0x00000064
    jalr x1, x1, 0      # 0x00000060

fail:
    j fail              # 20th ins from 0
    addi x2, x0, 0x60   # 0x00000060
    nop
    bne x1, x2, fail

pass:
    j pass