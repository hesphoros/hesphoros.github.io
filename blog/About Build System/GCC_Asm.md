# GCC 扩展内联汇编（extended asm）语法指南
本文件为面向内核/底层开发者的实用速查，介绍 GCC 的扩展内联汇编语法（"extended asm"），涵盖模板、输入/输出/读写操作数、约束字符串（constraints）、clobber 列表、asm goto、常见约束示例（x86、ARM）、以及调试与最佳实践。示例均以 GNU C / GCC 风格为准。

## 概览

## 常见示例

1. 简单输出：把 EFLAGS 读入变量（x86）

```c
# GCC 扩展内联汇编（extended asm）语法指南

本文件为面向内核/底层开发者的实用速查，介绍 GCC 的扩展内联汇编语法（"extended asm"），涵盖模板、输入/输出/读写操作数、约束字符串（constraints）、clobber 列表、asm goto、常见约束示例（x86、ARM）、以及调试与最佳实践。示例均以 GNU C / GCC 风格为准。

## 概览

GCC 的扩展内联汇编基本形式如下：

```c
asm ( "assembly template"
      : output_operands            /* optional */
      : input_operands             /* optional */
      : clobbers                   /* optional */
      );
```

更常见的写法带有 volatile 标识：

```c
__asm__ __volatile__("..." : ... : ... : ...);
```

说明：

- template：汇编代码文本，包含占位符 `%0`、`%1` 或命名占位符 `%[name]`。
- output_operands：输出操作数列表（如 `"=r" (out)`）。
- input_operands：输入操作数列表（如 `"r" (in)`）。
- clobbers：声明被修改但未作为输出的寄存器（如 `"cc"`, `"memory"` 或具体寄存器）。

模板中的占位符由输出先、输入后按顺序编号：第一个输出为 `%0`，第二个输出为 `%1`，第一个输入为随后编号，或使用命名占位符提高可读性。

## 输出 / 输入 / 读写 操作数

- 输出：`"=r" (result)` — `=` 表示写出。
- 输入：`"r" (value)` — 只读。
- 读写（in/out）：`"+r" (var)` — `+` 表示读并写。
- 早期覆盖（early-clobber）：`"=&r"` — 输出在所有输入读取之前就被覆盖，避免冲突。

示例（命名占位符）：

```c
int out;
int in = 5;
__asm__("add %[i], %[o]"
        : [o] "+r" (out)     /* out 为读写 */
        : [i] "r" (in)       /* in 为只读 */
        : "cc");             /* clobber condition codes */
```

## 约束字符串（constraints）要点

- 常见通用约束：
  - `r`：任意通用寄存器
  - `m`：内存操作数（编译器会提供地址/内存语法）
  - `i`：立即数常量
  - `g`：通用约束（寄存器/内存/立即数皆可）
- 平台特定（x86 常见）：`a,b,c,d,S,D,q` 等对应特定寄存器或寄存器子集。
- 修饰符：`&`（早期覆盖）、`+`（读写）、数字引用（`"0"(x)` 表示复用第 0 个操作数）等。

注：约束的具体可用符号随目标架构与 GCC 版本不同，务必参照 GCC 手册与目标 ABI。

## Clobbers（破坏说明）和内存/条件码

- `"cc"`：表示会改变条件标志（EFLAGS/RFLAGS）
- `"memory"`：表示会读写内存，阻止编译器对内存访问进行不安全的重排
- 具体寄存器（例如 `"rax"`）应列出在 clobber 中以避免编译器将其分配给其他变量

示例：简单自增并声明条件码已更改

```c
int foo;
__asm__("inc %0" : "+r"(foo) : : "cc");
```

## asm volatile 与非 volatile 的差别

- `asm volatile`：告诉编译器该汇编块有副作用，不可删除或任意移除/重排（除非有明确的 memory/clobber）。常用于 I/O、控制寄存器、屏障等。
- 非 volatile asm：编译器若判断无副作用可能会优化掉或合并该 asm 块。

## asm goto（从汇编跳转回 C 标签）

GCC 支持 `asm goto` 以实现从 asm 跳回 C 标签的能力，常用于实现错误/分支处理：

```c
asm goto ("cmp %0, %1; je %l[success]"
         : /* outputs */ : "r"(a), "r"(b) : /* clobbers */ : success);

success:
    /* 到达这里 */
```

使用 `asm goto` 会影响编译器对控制流的分析，必须小心测试所有路径。

## 常见示例

1. 简单输出：把 EFLAGS 读入变量（x86）

```c
unsigned long flags;
__asm__ volatile ("pushf; pop %0" : "=r" (flags));
```

2. 读-改-写（使用 + 或 early-clobber）：

```c
int x = 5, temp;
__asm__("xadd %0, %1"
        : "+r" (x), "=r" (temp)
        : "0" (x)
        : "cc");
```

3. x86 原子加（使用 lock 指令）：

```c
void atomic_inc(int *p) {
    __asm__ __volatile__("lock; incl %0"
                         : "+m" (*p)
                         :
                         : "cc");
}
```

4. asm goto 示例（伪代码）：

```c
int cond;
asm goto ("test %[c], %[c]\n    jnz %l[yes]"
          :
          : [c] "r" (cond)
          :
          : yes);

yes:
    /* 成功分支 */
```

## 平台快速参考（x86 / ARM）

- x86：支持很多具体寄存器约束（`a,b,c,d,S,D,q` 等），常用 `r,m,i`。
- ARM：常用 `r`（任意通用寄存器），`m`（内存），浮点/NEON 有特定约束（请参考 GCC 手册）。

## 调试与最佳实践

- 优先使用命名占位符 `%[name]` 提高可读性。
- 在不同优化级（`-O0`/`-O2`）测试 asm，观察寄存器分配变化。
- 对复杂序列，建议放到独立 `.S` 文件以降低 inline asm 与编译器交互复杂度。
- 明确列出 `memory` / `cc` / 寄存器 clobber，避免未定义行为。

## 参考

- GCC 官方文档（Extended Asm）：[GCC: Extended Asm](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html)

---

文件路径：`About Build System/GCC_Asm.md`

```text
简短说明：本稿旨在做为团队内部速查，涵盖常见使用场景与示例；若需要我可以继续：
- 添加更多架构（RISC-V、MIPS）示例；
- 把示例抽成 `examples/asm/` 可编译代码并在本地运行（包含 Makefile、测试用例）；
- 或把该文档翻译为英文并生成可打印版。
```
- 尽量使用命名占位符 `%[name]`，可读性更好且不易出错。
