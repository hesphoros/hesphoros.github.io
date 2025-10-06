# Branch predictive(分支预测化)

# 基本概念：什么是“分支”

程序中的条件语句、循环判断、本质上都会产生**控制流分支**（conditional branch）—— 比如 `if/else`、`switch`、`for/while` 的条件跳转。CPU 执行到分支指令时要决定下一条指令从哪儿来（执行true逻辑 or false逻辑）。

现代 CPU 是**深度流水线**（pipeline），如果等条件结果出来再继续，流水线会停顿，性能损失很大。于是出现了“先猜测后执行”的机制：**分支预测**。

# 分支预测（Branch Prediction）

**分支预测** 是 CPU 的硬件机制：在条件结果未确定前先预测走哪条路径，把那条路径的指令装进流水线。

常见组件与算法（摘要）：

- **BHT / PHT（Branch History Table / Pattern History Table）**： 用来记录分支历史（2-bit 饱和计数器很常见），用于局部/全局预测。
- **1-bit / 2-bit 预测器**：2-bit 比 1-bit 更稳健（避免因为一次异常改变预测）。
- **全局 vs 局部历史**：有的预测基于该分支本身的历史（局部），有的用全局最近分支历史（GShare、Global History）。
- **Tournament predictor（竞赛预测器）**：结合多种策略、选最合适的。
- **BTB（Branch Target Buffer）**：保存跳转目标地址（用于直接跳转/间接跳转）。
- **RAS（Return Address Stack）**：专门预测 `ret` 指令的返回地址。
- **间接分支预测**：比直接分支更难（indirect calls/jumps），需要更复杂的结构。

**代价（misprediction penalty）**：猜错后需要清空流水线并重新填充，代价是若干到数十个 CPU 周期（取决于流水线深度、micro-arch）。因此热点代码里错误预测会严重拖慢性

# “预测化” 的二义性

## A 分支预测提示（Hints，编译器提示）

程序员告诉编译器哪个分支“更常见”。常见形式：

```c++
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

if (LIKELY(x > 0)) { /* 常走 */ }
if (UNLIKELY(err)) { /* 罕见的错误处理 */ }

```

这只是对编译器的提示，编译器据此调整**代码布局**（把热路径放在一起）或生成不同的跳转序列，从而提高预测和缓存局部性。

## B. 预测化 

预测化 = **if-conversion / predication（去分支化 / 条件执行）**

这是编译器或程序员把有条件的控制流改为**条件式执行**或**无分支代码**，常见形式包括：

- 使用 `cmov`（x86 的条件移动）代替短分支。
- 使用三元运算符 / 算术运算把分支转为位运算或查表。
- 向量化（SIMD）把多个条件并行处理，避免逐条分支。
- 在某些架构（ARM old ISA）上有条件指令（conditional execution），可以执行预测化的代码路径。

if-conversion 的结果是 **消除分支**，从而避免分支预测失败造成的高昂代价。代价可能是执行更多指令或做无用工作，但在分支高度不可预测时通常更好。



# 编译器会做哪些优化（与分支相关）

**静态预测（static）**：基于规则（如编译器内置 heuristics）决定哪个分支更可能。

**使用 `__builtin_expect` 的 hint**：显式告诉编译器。

**代码布局 / 热/冷分割**：把常走路径放在一起，减少跳转和 I-cache miss。

**if-conversion（去分支化）**：把分支变成 `cmov` 或算术，从而消除条件跳转。

**函数内联 + 常量传播**：可能使条件成为常量，从而消掉分支。

**PGO（Profile-Guided Optimization）**：先运行收集真实运行时分支频率，再用这些数据生成最好的指令布局和分支实现。

**向量化/并行化**：把分支替换为无条件的向量指令流

#  常见的“去分支/预测化”手段（代码层）

**使用 `LIKELY/UNLIKELY` 把罕见路径标记出来**（如错误处理、断言失败）。

**用 `?:` / `cmov` / `std::min/std::max` 替代短的分支**（编译器可能自动转成 `cmov`）。

**查表（lookup table）** 替代复杂判断（当域有限时）。

**布尔算术**：`x = (cond ? a : b)` -> `x = b ^ ((a ^ b) & -cond)`（位技巧，特殊场景用）。

**批处理/向量化**：把多个判断一次处理，降低分支比例。

**把冷代码抽到单独函数 / cold section**（减少 I-cache 干扰）。

**减少复杂条件链**（简化判断逻辑，早返回把罕见分支放前面或后面视情形）

# 何时应该优化分支

- **先测量**。不要盲目优化。找到热点（hot path）并查看分支命中率。

- 优化目标：**真正高频且可测得的分支**。

- 用 `UNLIKELY` 标注异常/错误处理路径（罕见）往往安全且有益。

- 对高度不可预测的分支（随机或数据依赖强）考虑**去分支化**（表/算术/向量化）。

- 使用 **PGO** 或运行时采样优化布局效果最好。

- 小心：过度手工干预（错误的 `LIKELY`）可能反而降速。

# 如何衡量（工具和指标）

关键指标：**branch-misses**（分支预测失败次数）、**cycles**、**instructions**、**IPC**。常用工具：

- Linux: `perf stat -e branch-misses,branches,instructions,cycles ./a.out`
- Intel VTune、Linux perf record / report、FlameGraph 分析热点。
- Google Benchmark / microbenchmarks（注意避免测量伪造的分支行为）。

**测量要点**：用真实数据/长期负载或 PGO 采样，注意 CPU 频率、内存状态、冷启动 vs 热运行差别。

# example

## 1)使用hint

```c
if (UNLIKELY(ptr == nullptr)) {
    handle_error();
}
```

提示编译器：`ptr == nullptr` 很少发生，`handle_error()` 可被放到冷区

## 2)把短分支变为 `cmov`（伪示例）

```c++
// 分支版
int x = (a > b) ? a : b;

// 编译器可能生成 cmov 版本（无分支）

```

**3) 将分支去掉（查表示例）**

```c
// 假设 val 只有 0..3
int result = table[val]; // 替代多重 if/else

```

# 风险 / 其他注意事项

- **不要在不热的代码上滥用 `LIKELY/UNLIKELY`**。微调前先 profile。
- **分支消除有成本**：执行更多无用指令、寄存器压力、影响向量化等。
- **安全/投机执行影响**：硬件投机带来安全隐患（Spectre 等），在某些安全敏感场景需要更慎重的投机控制（操作系统/CPU 补丁/编译器选项）。
- **不同 CPU 行为不同**：同一优化在不同 micro-arch 上效果差异大（一定要在目标平台上测）。