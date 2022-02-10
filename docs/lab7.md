# 实验 7：综合实验

## 1 实验目的

- 学习 OS 在硬件层面的抽象
- 完善自己的 CPU Core，运行起一个简易 ELF 程序：Naive Kernel

## 2 实验环境

- **HDL**：Verilog
- **IDE**：Vivado
- **开发板**：Nexys A7
- :star2: **软件辅助环境**：[Docker Image](https://hub.docker.com/r/alphavake/oslab)（请选择 rv32 版本）

## 3 背景介绍

在系统二贯通课程的学习中，我们既学习了硬件系统（体系结构）也学习了软件系统（操作系统）。在这最后一个实验中，我们要尝试将一个简易 ELF 程序：Naive Kernel 运行在自己编写的 CPU Core 上。

为了降低实验难度，Naive Kernel 并不能算得上一个**真正**的操作系统，比如：

- Naive Kernel 只有一个特权态（Machine Mode）
- Naive Kernel 的中断栈是每个进程自己维护的
- Naive Kernel 进程出让时间片的方式是自己减少进程特有的 Counter，而并不是通过外部的时钟中断
- ...

Naive Kernel 不使用串口或其他协议与外界通信，而是使用 gp 寄存器来标志当前的运行状态：

- 当进程切换后，gp 的值更新为 `0x100 + task-id`
- 当处理了 `unimp` 指令带来的 `Illegal Instruction` 异常后，gp 的值更新为 mcause 的值（进阶版）

因此为了上板方便调试，推荐将 gp 寄存器的值输出到灯管上。

## 4 实验步骤

### 4.1 编译 Naive Kernel

本次实验提供了一个简单的Naive Kernel代码，实现的功能类似于`lab6`中的内容，初始化多个进程，待时间片耗尽后进行调度，在基础要求中只会发生调度导致的异常，按照`lab6`中的调度逻辑`trap_handler -> traps -> schedule -> switch_to -> __switch_to`顺序执行。

在编译得到的代码中会出现`auipc`指令，这是一条我们之前没有要求实现的指令，需要同学们在本次实验中补足，`auipc`指令的意义是计算`pc`加立即数的结果，可以参考`spec`手册实现。

由于上板和 QEMU 调试 Naive Kernel 需要不同的内存起始地址（上板的地址从 0x0 开始，QEMU 调试的地址从 0x80000000 开始），我们通过 lds 文件控制了加载地址。使用 make 默认编译出来的结果中的 `sim.elf` 是供 QEMU 调试用的 ELF。

由于本次实验的代码量较大，调试起来会有一定难度，建议同学们在出现问题时从某一条或者某一类指令可能出错分析，一定要先确保要求实现的指令都可以正常执行，再跟踪波形调试。

### 4.2 QEMU 运行与调试

Naive Kernel 不需要使用 OpenSBI 作为 Bootloader，因此在 QEMU 选项中使用 `-bios` 而非 `-kernel` 来加载 ELF 文件。因此为了调试 Naive Kernel，使用如下命令：

```bash
$ qemu-system-riscv64 -nographic -machine virt -bios path/to/sim.elf -S -s
```

其他流程均与之前的实验相似。

### 4.3 增加 CSR 寄存器

本次实验需要新增至少3个CSR寄存器：mtvec, mepc, mstatus，以保证程序的正确执行。CSR寄存器的长度为MXLEN-bit，在本次实验中取MXLEN=32。我们推荐将CSR寄存器封装在一个独立的CSR模块（module）中，将实现的CSR寄存器通过接口与流水线CPU进行交互。

下面是3个寄存器的简要介绍，我们在实验中只要求实现某些寄存器的部分功能，如果想要了解完整的处理机制可以参考[RISCV官网](https://github.com/riscv/riscv-isa-manual/releases/download/Priv-v1.12/riscv-privileged-20211203.pdf)。

1. mtvec(Machine Trap-Vector Base-Address Register)寄存器是可读可写寄存器，存储异常处理代码的地址，本次实验只需要实现`Direct`模式，即发生异常时跳转到mtvec所指向的地址进行处理。

2. mepc(Machine Exception Program Counter)寄存器是可读可写寄存器，存储发生异常时的地址。

3. mstatus(Machine Status Register)寄存器是可读可写寄存器，存储M模式下的异常相关的信息，在本次实验中我们只需要实现其中的`MIE(3)`即可。


### 4.4 增加特权态指令

1. csrr\[w/s/c\]\[i\]是对于CSR寄存器进行读写操作的指令，这里以`csrrw rd, csr, rs`举例说明，csrrw的指令含义是Atomic Read/Write CSR，原子地读写CSR寄存器，即原子性地完成`gpr[rd] = csr, csr = gpr[rs]`一读一写两个操作。与之相类似，csrrs即set操作`gpr[rd] = csr, csr = csr | gpr[rs]`，csrrc即clear操作`gpr[rd] = csr, csr = csr & ~gpr[rs]`，\[i\]即为立即数指令，更多相关的细节可以参考[RISCV官网](https://github.com/riscv/riscv-isa-manual/releases/download/Ratified-IMAFDQC/riscv-spec-20191213.pdf)。

2. ecall指令的含义是向执行环境发出请求，我们在`lab4~6`中实现的`sbi_ecall`就是通过`ecall`向权限更高的M模式发送请求，完成类似于打印字符，设置时钟计时等。在本次实验中，我们只实现M模式，在执行指令`ecall`后会触发异常`Environmrnt call from M-mode`，需要对相关寄存器进行设置并跳转到异常处理地址。

3. mret指令的含义是从M模式的Trap下返回，在本次实验中，我们只实现M模式，因此无需考虑所处模式的变化，在执行指令`mret`后会从Trap中返回到正常的代码执行流中，即寄存器`mepc`所储存的地址。（请思考当发生异常和中断时，返回地址有什么区别）


### 4.5 增加异常处理逻辑

异常处理逻辑与控制跳转类似，相似点是以流水线内部的视角来看都是跳转到一个地址继续执行，只不过异常是跳转到mtvec寄存器所在的地址，不同之处在于CSR模块内部的处理，异常会导致相关的CSR寄存器状态发生变化。在本次实验中，我们只实现M模式，以及最基本的三个CSR寄存器，当发生`ecall`异常时，会导致`mepc`寄存器发生变化，当执行`mret`时，需要返回`mepc`所指向的地址。

这里要注意异常的机制设计，先思考好异常在哪个阶段判断，在异常发生时哪些指令应该被执行，哪些指令不该被执行，以及流水线的刷新，传递等问题。

CSR模块的基本功能是对CSR寄存器进行读写，需要的接口即为`csr_write`使能, `addr`CSR编号, `din, dout`数据端口。考虑到`ecall`命令执行时需要跳转到`mtvec`并将当前的`pc`写入`mepc`，因此还需要`ecall`信号端口，`pc`输入端口和`mepc`写端口，如果实现了高阶的功能则还需要增加`mcause`的写端口。

### 4.6 波形仿真调试

本次实验中我们将测试文件的指令数提升了若干倍，也使得调试难度增大了许多，我们给出以下几点参考建议：

1. 可以通过`qemu`等方式理解程序的执行过程，在调试过程中可以先把`COUNTER_INIT`变量缩减为`0x5`，防止波形过长。
2. 实验中`csr[c/s/w]`指令较少，在调试时可以先关注`_start`函数中该指令的正确性。
3. 在调试时可以手动跟踪第一次由task0切换到task1的全过程，这个过程不算特别长，可以手动查看执行流与关键寄存器的值，这里比较重要的寄存器是`a5`，在执行到函数`proc`中时，`a5`寄存器存储的是`current->counter`，可以通过`a5`的变化大致观察到程序运行过程。确保上述过程正确后再添加`gp`寄存器查看，`gp`寄存器存储的是`0x100 + task_id`，观察是否是从小到大循环变化，即可验证仿真是否通过。
4. 实现时需要同学们考虑RAM的大小问题，本次实验的数据段地址位于`[0x500, 0x500 + 0x100 * task_id]`，需要扩大RAM的大小，请同学们调整好相关的宽度问题。
5. 注意在进程第一次被调度时不会修改`gp`的值。

## 5 实验步骤（进阶）

### 5.1 编译 Naive Kernel Advanced Version

编译原理与普通版相同，在Makefile文件中加入`-DADVANCE`选项即可。进阶版在`counter = 0x555`时会执行一条未定义指令，触发异常后`kernel`会通过软件将`gp`的值改写为`mcause`的值，即不需要通过硬件实现这一要求，只需要实现新增的CSR寄存器和异常判断逻辑即可。

### 5.2 增加 CSR 寄存器

mcause(Machine Cause Register)寄存器保存了发生异常的原因，当发生了变化为M模式处理的异常时，mcause寄存器应当写入发生异常的原因，在privileged手册中可以查看RISCV对不同的`Interrupt`和`Exception`规定的编号。

### 5.4 增加异常处理逻辑

illegal instruction：当读取到非法指令时，触发该异常，跳转至mtvec，保存mepc和mcause。


## 作业提交



基本要求：实现M特权态，增加指令`ecall, mret, csr/[s/w/c/]`，增加CSR寄存器`mtvec, mepc, mstatus`，实现异常机制

高阶要求：编译运行ADVANCE kernel，增加CSR寄存器`mcause`，增加`illegal instruction`异常
