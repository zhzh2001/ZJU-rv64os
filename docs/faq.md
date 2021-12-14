# 常见问题及解答

## 1 仿真和上板结果对不上怎么办

往往是一些 Verilog 代码的细节不当，导致了综合器无法综合出与你预期的代码。

首先可以尽可能地消除你的工程中的 Warnings。此外，可以参考这份 [slides](http://www.ee.ncu.edu.tw/~jfli/vlsidi/lecture/Verilog-2012.pdf) 的 17-31 页检查你的工程。

## 2 Linux 源码放在共享文件夹下编译失败

这种情况请使用 `wget` 等工具将 Linux 源码下载至容器内目录**而非共享目录**，然后执行编译。

## 3 QEMU+GDB 使用 `si` 单指令调试遇到模式切换失效

在遇到诸如 `mret`, `sret` 等指令造成的模式切换时，`si` 指令会失效，可能表现为程序开始不停跑，影响对程序运行行为的判断。

一个解决方法是在程序**预期跳转**的位置打上断点，断点不会受到模式切换的影响，比如：

```bash
(gdb) i r sepc    
sepc        0x8000babe
(gdb) b * 0x8000babe
Breakpoint 1 at 0x8000babe
(gdb) si    # 或者使用 c
Breakpoint 1, 0x000000008000babe in _never_gonna_give_you_up ()
...
```

这样就可以看到断点被触发，可以继续调试了。

## 4. Function Prologue and Epilogue in C

在实验中C语言编译出的函数会多出一部分非程序的代码，集中在函数的开头与结尾，这被称作`Prologue`和`Epilogue`，他们起到了分配栈空间等作用，具体的信息可以查看这个[回答](https://stackoverflow.com/questions/14765406/function-prologue-and-epilogue-in-c)。

## 5. mtime & mtimecmp

在QEMU中也可以方便地查看`mtime`和`mtimecmp`的值，`mtime`和`mtimecmp`的实现是通过MMIO(Memory-mapped I/O)的方式实现的，在QEMU的默认设置中mtime的地址位于0x200bff8，读这个地址的值就是mtime的值(我们的实验中是一个64bit的量)，mtimecmp的地址在0x2004000。
