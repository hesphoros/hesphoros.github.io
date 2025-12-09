# GDT（全局描述符表）——实模式与保护模式介绍

本文件面向内核与引导程序作者，介绍 GDT 的基本概念与使用方法，比较实模式（Real Mode）与保护模式（Protected Mode）中段机制的差异，并给出构造 GDT 描述符、加载 GDT（LGDT）、切换到保护模式（设置 CR0.PE）以及同步段寄存器（CS/DS/ES 等）的示例代码（C/汇编）。最后包含 TSS/LDT、64-bit（Long Mode）相关注意事项与调试技巧。

## 快速概览

- GDT（Global Descriptor Table）是 x86 提供的一张描述符表：每个描述符定义段的基址、界限、访问权限和标志。GDT 本身在实模式下通常不被使用，保护模式下用于段选择与访问控制。
- 选择子（Selector）是 16 位值：索引（高位）<<3 + TI(1 bit) + RPL(2 bit)。通过选择子将段寄存器（如 CS、DS）与 GDT 中对应描述符关联。
- 在引导过程中常见步骤：构建 GDT（包含 null descriptor、代码段、数据段），用 LGDT 加载 GDTR，设置 CR0.PE=1（进入保护模式），然后执行一个远跳转（far jump）以刷新 CS 并使处理器使用新的段描述符。

## 实模式 vs 保护模式（重点差异）

- 实模式（Real Mode）
  - 段机制简单：物理地址 = 段寄存器 * 16 + 偏移。并不使用 GDT 描述符格式。
  - 没有硬性的段限界检查；段寄存器仅保存 16 位段基值（隐含左移 4）。
  - BIOS 中断与传统实模式接口都默认实模式段模型。

- 保护模式（Protected Mode）
  - 使用描述符（GDT/LDT）指定段的基址、段界限与访问权限（读/写/执行、特权级等）。
  - 段寄存器（CS/DS/ES/SS/FS/GS）保存选择子；处理器在访问内存时根据选择子查找描述符并进行界限/权限检查。
  - 支持分页（与段机制联合使用）以及特权级别（CPL/RPL）和任务切换（TSS/LDT）。

实模式到保护模式的常见流程：
1. 在实模式或临时实模式下建立一份 GDT（放在内存中）。
2. 加载 GDTR（LGDT [gdtr]）。
3. 设置 CR0.PE = 1（把控制寄存器 CR0 的最低位 PE 置 1）。
4. 执行一个 far jump 到新代码段选择子（刷新 CS 并开始用新段描述符）。
5. 在保护模式下设置数据段寄存器（MOV DS, sel; MOV SS, sel 等）。

## GDT 描述符结构（32-bit 段描述符）

每个段描述符为 8 字节，字段（按字节序列描述，低位在前）：

- limit (15:0)         — 描述符低 16 位的界限
- base  (15:0)         — 描述符低 16 位的基址
- base  (23:16)        — 基址中间 8 位
- access byte (8 bit)  — 存取字节（存在位、可执行位、描述符特权级 DPL、已访问等）
- limit(19:16) + flags — 界限高 4 位 与 标志（G 位: 4K 粒度, D/B: 默认操作大小, L: 64-bit 代码段位）
- base  (31:24)        — 基址高 8 位

字段说明：
- Access 字节位含义（常见）:
  - P (bit7) — Present（存在位）
  - DPL (bit5-6) — Descriptor Privilege Level（特权级）
  - S (bit4) — Descriptor type (0=system, 1=code/data)
  - Type (bit0-3) — 对于 code/data：是否可读/写、是否为可执行等

- Flags 字节高 4 位（与 limit 的高 4 位同字节）：
  - G (bit7 of byte) — Granularity（0: 单位字节, 1: 单位 4KB）
  - D/B (bit6) — Default operation size (0=16-bit, 1=32-bit) 或者 B 表示扩展/默认大小
  - L (bit5) — 64-bit code segment (Long mode)（仅在 IA-32e 使用）

示例：创建一个常见的 32-bit 平坦模型代码段（基址=0, limit=0xFFFFF, G=1, D=1, Access=0x9A）和数据段（Access=0x92）：

```c
/* C 风格 GDT 条目结构 */
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  limit_high_flags;
    uint8_t  base_high;
} __attribute__((packed));

struct gdtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void set_gdt_entry(struct gdt_entry* e, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    e->limit_low = limit & 0xFFFF;
    e->base_low = base & 0xFFFF;
    e->base_mid = (base >> 16) & 0xFF;
    e->access = access;
    e->limit_high_flags = ((limit >> 16) & 0x0F) | (flags & 0xF0);
    e->base_high = (base >> 24) & 0xFF;
}

/* 常见的 access/flags 值 */
/* Code segment: 0x9A = present, ring0, code, executable, readable */
/* Data segment: 0x92 = present, ring0, data, writable */
/* flags: 0xC0 -> 1100 0000 : G=1 (4KB granularity), D/B=1 (32-bit) */
```

注：`flags` 这里的高 4 位映射为 G/D/L 等；`set_gdt_entry` 组合时把高 4 位加到 limit 的高半字节中。

## 选择子（Selector）格式

选择子 16 位结构：

- Bits 15..3 — Index（描述符表索引）
- Bit 2 — TI（Table Indicator：0=GDT, 1=LDT）
- Bits 1..0 — RPL（Requested Privilege Level，申请的特权级）

例如：GDT 中第 1 项（index=1）作为代码段，选择子 = (1 << 3) = 0x08。通常我们把 GDT 的第 0 项保留为 null descriptor（所有位 0），第 1 项为内核代码段（selector 0x08），第 2 项为内核数据段（selector 0x10）。

## LGDT / 切换到保护模式（典型汇编示例）

下面给出一个经典的引导器片段，展示如何在实模式下准备 GDT 并切换到保护模式（注意：示例以 NASM 语法为例）：

```asm
; 假设 gdt_descriptor 已经在数据区定义好，格式为 [limit (2 bytes)] [base (4 bytes)]
    cli                 ; 关中断
    lgdt [gdt_descriptor]

    ; 设置 CR0.PE = 1
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 远跳转刷新 CS，并进入保护模式的代码段 (selector = 0x08)
    jmp 0x08:protected_entry

protected_entry:
    ; 现在处于保护模式（CPL = 0），可以设置数据段寄存器
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ; 继续内核初始化
```

注意点：
- 在设置 CR0.PE 之前必须确保 IDT/GDT 与堆栈等关键数据就绪；通常先加载 GDT 后再置 PE。
- 远跳转的目的：刷新指令预取并让处理器用新的 CS（基于 GDT 中的描述符）来获取段基与权限信息。

## 示例：在 C 中加载 GDT（内联汇编）

```c
extern void lgdt(void*);
struct gdtr mygdtr;
mygdtr.limit = sizeof(gdt) - 1;
mygdtr.base = (uint32_t)&gdt;
asm volatile ("lgdt (%0)" : : "r" (&mygdtr));

/* 设置 CR0.PE */
unsigned int cr0;
asm volatile ("mov %%cr0, %0" : "=r" (cr0));
cr0 |= 1;
asm volatile ("mov %0, %%cr0" : : "r" (cr0));

/* 远跳转到新代码段（用汇编执行） */
asm volatile ("jmp $0x08, $protected_continue");

/* 然后在 protected_continue 处设置数据段寄存器 */
```

## LDT / TSS 简介

- LDT（本地描述符表）允许为特定任务/进程定义私有段表，但现在很少在现代内核中使用。
- TSS（任务状态段）用于硬件任务切换（现代 OS 多采用软件任务切换），也用于保存在特权级切换时的栈指针（例如当发生从用户态到内核态的中断，CPU 会使用 TSS 中的 SS:ESP 为内核栈）。
- TSS 本身在 GDT 中以系统段描述符（S=0）存在，使用 LTR 指令加载任务寄存器（TR）。

示例：加载 TSS

```asm
    ; 假设 tss_selector 在 GDT 中已设置
    ltr ax  ; ax = tss_selector
```

## 基于 OSDev 教程的实用示例

下面摘取并改写自 OSDev 的 GDT 教程，提供一组方便的宏与运行/编码示例，便于在引导器或早期内核中构建 GDT。

```c
// 辅助宏与示例代码（C）
#include <stdio.h>
#include <stdint.h>

// 各位字段的宏（参考 Intel 文档）
#define SEG_DESCTYPE(x)  ((x) << 0x04) // Descriptor type (0 for system, 1 for code/data)
#define SEG_PRES(x)      ((x) << 0x07) // Present
#define SEG_SAVL(x)      ((x) << 0x0C) // Available for system use
#define SEG_LONG(x)      ((x) << 0x0D) // Long mode (64-bit code)
#define SEG_SIZE(x)      ((x) << 0x0E) // Size (0 for 16-bit, 1 for 32)
#define SEG_GRAN(x)      ((x) << 0x0F) // Granularity (0: byte, 1: 4KiB)
#define SEG_PRIV(x)     (((x) &  0x03) << 0x05) // Privilege level (0..3)

// 常用类型定义（code / data）
#define SEG_DATA_RD        0x00 // Read-Only
#define SEG_DATA_RDA       0x01 // Read-Only, accessed
#define SEG_DATA_RDWR      0x02 // Read/Write
#define SEG_DATA_RDWRA     0x03 // Read/Write, accessed

#define SEG_CODE_EX        0x08 // Execute-Only
#define SEG_CODE_EXRD      0x0A // Execute/Read

// 组合常见段描述符（平坦模型，Ring 0）
#define GDT_CODE_PL0 (SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_LONG(0) | SEG_SIZE(1) | SEG_GRAN(1) | SEG_PRIV(0) | SEG_CODE_EXRD)
#define GDT_DATA_PL0 (SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_LONG(0) | SEG_SIZE(1) | SEG_GRAN(1) | SEG_PRIV(0) | SEG_DATA_RDWR)

// 把 base/limit/flags/ACCESS 编码成 64-bit 描述符（便于调试或在用户态程序生成）
uint64_t make_descriptor(uint32_t base, uint32_t limit, uint16_t flag) {
  uint64_t descriptor;

  // high 32 bit: 包含 limit(19:16), flags/access, base(23:16), base(31:24)
  descriptor  =  (limit  & 0x000F0000ULL) >> 16;
  descriptor |= ((uint64_t)flag << 8) & 0x00F0FF00ULL;
  descriptor |= ((uint64_t)(base >> 16) & 0x000000FFULL);
  descriptor |= ((uint64_t)(base & 0xFF000000ULL));

  descriptor <<= 32;

  // low 32 bit: base(15:0) << 16 | limit(15:0)
  descriptor |= ((uint64_t)(base & 0xFFFF) << 16);
  descriptor |= (limit & 0x0000FFFFULL);

  return descriptor;
}

int main(void) {
  printf("0x%.16llX\n", make_descriptor(0, 0, 0));
  printf("0x%.16llX\n", make_descriptor(0, 0x000FFFFF, GDT_CODE_PL0));
  printf("0x%.16llX\n", make_descriptor(0, 0x000FFFFF, GDT_DATA_PL0));
  return 0;
}
```

如果需要在引导/内核早期按字节填充 GDT 条目，可使用按字节编码函数（更适合在裸机环境中直接写入内存）：

```c
void encodeGdtEntry(uint8_t *target, uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t flags)
{
  if (limit > 0xFFFFF) {
    // 限界无法编码
    return;
  }

  // limit low
  target[0] = limit & 0xFF;
  target[1] = (limit >> 8) & 0xFF;
  target[6] = (limit >> 16) & 0x0F;

  // base
  target[2] = base & 0xFF;
  target[3] = (base >> 8) & 0xFF;
  target[4] = (base >> 16) & 0xFF;
  target[7] = (base >> 24) & 0xFF;

  // access & flags
  target[5] = access_byte;
  target[6] |= (flags << 4) & 0xF0;
}
```

这些工具在引导器中非常有用：既可在 C 中生成整型描述符写入表，也可以按字节编码以精确控制内存布局。

## 进入 Long Mode（x86_64）时的注意事项

- 进入 IA-32e/Long Mode 需要先进入保护模式，再启用 PAE，设置 EFER.LME（MSR），最后切换到长模式。GDT 在其中仍然需要：
  - 长模式下的代码段描述符要求 L 位（64-bit）为 1 且 D/B = 0。
  - 大多数现代内核在进入长模式前会使用一个简洁的 GDT（null、64-bit code、compat code/data）来建立一致的段环境。
- 在长模式下数据段基通常视为 0（flat），并且大多数段限界/保护机制由分页（EPT/CR3 管理的页表）负责。

## 常见陷阱与调试技巧

- 忘记 null descriptor：GDT 的第 0 项必须为全 0（防止使用未初始化的选择子）。
- GDTR 的 limit 必须是表字节数减 1（`sizeof(gdt)-1`）。
- GDT 必须在内存中对齐；虽然并非严格要求 8 字节对齐，但为了性能与兼容性建议按 8 字节对齐。
- 远跳转（far jump）必须在设置 CR0.PE 后立即执行以刷新 CS；否则 CPU 仍使用旧的段选择。
- 在 QEMU/Bochs 中调试：在切换点前设置断点，检查 CR0/CR3/CS 寄存器和 GDTR 内容（使用 GDB 或 QEMU monitor）。

## 参考资料

- Intel® 64 and IA-32 Architectures Software Developer’s Manual — Volume 3 (System Programming Guide). 这是权威参考。
- OSDev Wiki — GDT 入门与示例：[OSDev GDT](https://wiki.osdev.org/GDT)
- [全局描述符表 - 维基百科 --- Global Descriptor Table - Wikipedia](https://en.wikipedia.org/wiki/Global_Descriptor_Table)
- [实模式与保护模式 - seineo's blog](https://seineo.github.io/实模式与保护模式.html)

![img](https://upload.wikimedia.org/wikipedia/commons/thumb/0/0a/SegmentDescriptor.svg/580px-SegmentDescriptor.svg.png)
