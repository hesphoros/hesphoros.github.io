# PIC 与 PIE 简明指南

本文解释 Position-Independent Code (PIC) 与 Position-Independent Executable (PIE) 的概念、实现机制、平台差异、常用编译/链接选项与测试调试方法，并给出具体示例（x86_64 与 i386）。

## 概念速览

- PIC（Position-Independent Code）: 可重定位、在运行时无需修改自身代码就能在任意地址执行的代码，常用于共享库（shared libraries）。
- PIE（Position-Independent Executable）: 可重定位的可执行文件，本质上是一个使用 PIC 技术生成的可执行程序（但链接为可执行而非共享库），常用于支持 ASLR（地址空间布局随机化）的可执行文件。

简言之：PIC 是一种代码生成风格；PIE 是把使用该风格生成的可执行文件（可被加载到随机地址）。

## 为什么需要 PIC/PIE

- 共享库加载到任意地址而无需重写可执行段，实现代码共享与节省内存。
- PIE 允许内核在每次启动或每次进程创建时以随机地址加载可执行文件，从而提高安全性（ASLR）。
- 在某些平台/模式（比如动态链接、沙箱和某些 OS 加固策略）下，PIE 已成为默认或强制要求。

## 基本实现思路

实现 PIC 的常见技术包括：

- 使用相对寻址（x86_64 的 RIP-relative addressing）以避免绝对地址常量。
- 使用全局偏移表 (GOT) 存储全局变量与外部符号的运行时地址；代码通过 GOT 的相对引用来访问数据。
- 使用过程链接表 (PLT) 和 GOT 的结合来延迟绑定函数地址（动态链接时用于跨模块函数调用）。
- 链接器在生成共享库或 PIE 时会生成重定位表（relocations），在加载/动态链接阶段修正 GOT/relocs。

不同架构实现细节不同：x86_64 推荐使用 RIP-relative（对小常量/地址）来实现 PIC；而 32-bit x86 常依赖 GOT 表来实现数据/符号访问（因为没有 native RIP-relative 机制）。

## x86_64（64-bit）关键点

- 编译器可以生成 RIP-relative 指令（例如 `mov rax, [rip + offset]`），这是一种天然的 position-independent 访问方式，不需要额外的 GOT 条目来访问小的静态数据或指针表。
- 对于外部符号或复杂情况，GOT/PLT 仍然被使用，但总体来说 x86_64 的 PIC 比 i386 更高效。

示例：RIP-relative 访问（伪汇编）

```asm
; x86_64: load address of .data_symbol into rax
mov rax, QWORD PTR [rip + .L_got_pc_relative_data]
; 调用时根据 GOT/PLT 解析函数地址
call QWORD PTR [rip + .L_got_function]
```

## i386（32-bit x86）关键点

- 32-bit x86 没有 RIP 寄存器，通常依赖 GOT：编译器把对全局/外部符号的访问转换为通过 `%ebx`（或其它寄存器）指向 GOT base 的偏移访问。
- 常见做法是：在函数入口设置 `%ebx` 指向 GOT 的基址，然后通过 `%ebx`+offset 访问全局数据。

伪汇编示例（i386 PIC）

```asm
; 函数入口由编译器插入的代码：
call 1f
1: popl %eax            ; 获取当前地址到 eax
subl $_GLOBAL_OFFSET_TABLE_ - (.-1b), %eax
movl %eax, %ebx        ; %ebx = GOT base
; 之后访问全局变量用 (%ebx + offset)
```

## 编译与链接选项（GCC / Clang）

常用选项：

- 生成 PIC（用于共享库）:
  - `-fPIC`（或 `-fpic`） —— 生成位置无关代码（`-fPIC` 在某些平台会生成更通用/大范围的 GOT 访问，适合大型对象）。
- 生成 PIE（用于可执行文件）:
  - `-fPIE`（或 `-fpie`） 在编译时启用 PIC 风格生成，
  - `-pie` 在链接阶段告诉链接器生成可重定位可执行文件（PIE）。
- 生成共享库：
  - `-shared -fPIC -o libfoo.so foo.o`

示例（PowerShell / cmd 下）:

```powershell
# 构建共享库 (x86_64 linux example)
gcc -fPIC -O2 -c foo.c -o foo.o
gcc -shared -o libfoo.so foo.o

# 构建可执行 PIE
gcc -fPIE -O2 -c main.c -o main.o
gcc -pie -o app main.o -L. -lfoo
```

> note: 在某些平台上 `-fpic` 也可用，区别是 `-fPIC` 对应更通用、更大范围的重定位处理（适用于大型共享库）；`-fpic` 产生更小/限制更多的重定位形式。

## 链接器与运行时（动态链接器）

- 对于共享库（.so），动态链接器在加载时会解析重定位表并填充 GOT（lazy binding 或 eager binding）。
- 对于 PIE，可执行文件也会包含重定位记录；内核或动态链接器负责在加载进程映像时应用重定位，使得程序可以在随机地址执行。
- 使用 `readelf -r` / `objdump -R` 可查看重定位条目；`readelf -s` 可查看符号表；`ldd` 可查看动态依赖。

## 性能与大小影响

- PIC/PIE 需要额外的间接访问（通过 GOT/PLT），这会引入轻微的性能开销（更多的内存引用、间接跳转），尤其在 i386 上更明显。
- x86_64 的 RIP-relative 通常开销更低，但依然可能增加重定位记录尺寸。
- 对于性能敏感的代码（如内核/低层库），有时会为关键路径选择非 PIC 实现或使用特定编译器/链接优化。

## 安全性

- PIE 支持 ASLR，使得相同的可执行文件在不同运行或每次启动时被加载到不同地址，从而增加利用难度。
- 现代 Linux 发行版往往把默认生成的二进制设置为 PIE（或在打包时强制 PIE）。

## 实验与调试命令（在 Linux 下）

```powershell
# 查看一个对象的重定位表
readelf -r foo.o

# 查看可执行或共享库的符号表
readelf -s libfoo.so

# 反汇编查看 GOT/PLT 用法
objdump -d -M intel libfoo.so | less

# 测试是否为 PIE（ELF type）
readelf -h app | Select-String "Type:" -SimpleMatch

# 在运行时观察 ASLR 效果
Get-Process -Name app  # (Windows example); on Linux use ps and cat /proc/<pid>/maps
```

> 注意：PowerShell 下 `readelf`/`objdump` 若不可用，需在 Linux 环境或安装对应的 binutils。QEMU + cross-compiler 也常用于交叉测试。

## 示例：从 C 构建并验证

1. 创建 `foo.c`（库）与 `main.c`（使用库）。
2. 使用上面的 `gcc -fPIC` / `-shared` 生成 `libfoo.so`。
3. 编译 `main.c` 为 PIE（`-fPIE -pie`），并运行 `ldd app`、`readelf -r app` 来确认重定位与依赖。

## 汇编角度小结与注意事项

- 当你用汇编编写需要作为共享对象或 PIE 的代码时：
  - 在 x86_64 上优先使用 RIP-relative 指令访问数据/符号；
  - 在 i386 上，遵循编译器生成的 GOT/PLT 模式，或直接使用编译器把你的汇编包装为 `.S` 并让编译器处理 GOT 基址初始化；
  - 注意函数指针与数据指针访问的间接性（间接加载可能需要额外寄存器来保存 GOT 基址）。

## 小结

- PIC 是代码生成技术，使代码可在任意地址运行；PIE 是采用 PIC 技术的可执行文件，常用于 ASLR。
- x86_64 提供 RIP-relative 支持，使 PIC 更高效；i386 更依赖于 GOT/PLT。编译器提供 `-fPIC/-fPIE`，连接器通过 `-shared` / `-pie` 生成目标。
- 对调试、性能影响与平台特性要有清晰认识，必要时针对关键路径进行特殊处理。

