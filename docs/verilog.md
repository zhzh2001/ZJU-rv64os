# Verilog Tips

## 资源

* [Verilog Standard: IEEE 1364 - 2005](https://ieeexplore.ieee.org/document/1620780)
    * Verilog 语言标准，当你想学习 Verilog 语言技巧，对语言本身产生困惑时可以看。比如它能解答这些疑问：
        * 用小于号比较两个 `reg` 到底是有符号比较还是无符号比较？
        * 如何使用 hierarchical names 来方便流水线 CPU 的开发？
        * 如何使用 `generate` 来避免大量重复的代码？



## Bad Practice & Good Practice

### (Bad) 混合同步与异步更新

下面这段代码**不能**达到想要的效果：

``` verilog
// bad practice
always @(posedge clk or posedge rst) begin
    if (rst || have_exception) begin
    	<some code A> 
    end
end
```

有人会这样写是因为 TA 发现 `rst` 和 `have_exception` 时要做的动作是相同的，与其写两个一样的分支，不如合并起来写。但这样在 Vivado 里会带来非常奇怪的现象，因为我们把异步和同步混合起来了—— `rst` 一般代表异步更新，而我们希望 `have_exception` 的分支同步于 `clk` 起作用。正确的写法就是拆开来：

``` verilog
// good practice
always @(posedge clk or posedge rst) begin
    if (rst) begin
        <some code A>
    end else if (have_exception) begin
        <some code A>
    end
end
```

