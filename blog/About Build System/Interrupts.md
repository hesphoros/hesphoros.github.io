# 中断（Interrupts）与实现 IDT

本文档以 OSDev Wiki（参考：Interrupts / IDT）为基础，给出实现 IDT 的理论背景、数据结构、初始化步骤与可复制的代码示例（包含 PIC 重映射、ISR/IRQ stub、C 处理链与注册接口）。目标读者：实现一个简单的 x86 保护模式内核并能接收外设 IRQ 的内核开发者。

> 注：本文使用遗留 BIOS/实模式启动到保护模式后的环境示例，汇编示例采用 NASM/GAS 风格注释并给出对应的 C 接口。若使用 UEFI 或 64-bit long mode，需做相应调整（IDT 描述符与堆栈语义存在差异）。

## 概念回顾

- 中断（interrupt）：CPU 在执行指令流中遇到的同步（异常）或异步（外设 IRQ）事件，需要临时保存上下文并跳转到处理例程。
- 异常（exceptions）：CPU 产生的同步中断，例如除零、缺页等，编号通常为 0–31。
- IRQ（hardware interrupts）：外设触发的中断，经 PIC/APIC 转发，传统 x86 PIC IRQ 编号为 0–15（经过重映射通常映射到 IDT 向量 32–47）。
- IDT（Interrupt Descriptor Table）：一张描述符表，CPU 通过向量（0–255）索引表项，使用该描述符进行控制转移（中断门/陷阱门/任务门）。

## IDT 条目（32-bit Protected Mode）

在 32-bit 保护模式下，IDT 条目（interrupt gate）大小为 8 字节，布局通常如下（C 结构示例）：

```c
// 32-bit IDT entry (interrupt gate)
struct idt_entry {
    uint16_t base_lo;   // handler 地址低 16 位
    uint16_t sel;       // 代码段选择子（通常内核代码段）
    uint8_t  always0;   // 保留, 必须为 0
    uint8_t  flags;     // 类型和属性（P/DPL/类型）
    uint16_t base_hi;   // handler 地址高 16 位
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base; // IDT 表的线性地址
} __attribute__((packed));
```

flags 字段常用值：0x8E 表示 P=1, DPL=0, type=0xE（32-bit Interrupt Gate）。

## IDT 初始化步骤概要

1. 在内核中为 IDT 分配一个 256 条目的数组（或动态分配需要的数量），并将所有条目标记为默认（例如指向一个 panic/未处理处理器）。
2. 为 CPU 异常（0–31）安装对应的 ISR stub（汇编：保存寄存器并跳转到通用的 C 处理器入口）。
3. 重映射 PIC（可选但常用）以避免 IRQ 与异常向量冲突：将主/从 PIC 的向量基址设置为 0x20/0x28（即 IRQ 0 映射为向量 32）。
4. 为 IRQ（通常 IRQ0..IRQ15 映射到 IDT 向量 32..47）安装 IRQ stub。
5. 使用 lidt 指令加载 IDT 表基址与 limit。
6. 在准备就绪后使能中断（sti）。

## PIC 重映射（示例代码）

传统 8259 PIC 的默认向量会与 CPU 异常重叠，常做法是把主从 PIC 重映射到 0x20/0x28。下面是常用的重映射实现（使用端口 I/O）：

```c
#include <stdint.h>
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

static inline void outb(uint16_t port, uint8_t val);
static inline uint8_t inb(uint16_t port);

void pic_remap(int offset1, int offset2) {
    uint8_t a1 = inb(PIC1_DATA); // 保存掩码
    uint8_t a2 = inb(PIC2_DATA);

    // 初始化 PIC
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);
    // 设置向量偏移
    outb(PIC1_DATA, offset1);
    outb(PIC2_DATA, offset2);
    // 告诉主从 PIC 的连接关系
    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);
    // 设置 8086 模式
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // 恢复保存的掩码
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}
```

在初始化时常用的调用：

```c
pic_remap(0x20, 0x28); // 将 IRQ 0-15 映射到 IDT 向量 32-47
```

## ISR/IRQ Stub 与通用 C 处理器入口

一个常见结构：每个中断/IRQ 有一个小的汇编 stub，功能是保存必要寄存器并把控制传给一个 C 函数（例如 isr_handler 或 irq_handler）。示例（NASM 风格伪代码）：

```nasm
; isr_stub.nasm
global isr_common_stub

isr_common_stub:
    pusha                   ; push general-purpose registers
    push ds
    push es
    push fs
    push gs
    ; load kernel data segments if needed
    ; push interrupt number (call-site或通过 separate stubs 推入常量)
    push dword [esp + 24]   ; (示意) 中断号传递到 C
    call isr_handler_c
    add esp, 4              ; 清理参数
    pop gs
    pop fs
    pop es
    pop ds
    popa
    iret

; 更常见的做法是为每个中断生成一个小的 stub，直接 push 常量中断号，然后 jmp isr_common_stub
```

对应的 C 处理器入口示例：

```c
// registers_t 表示在 stub 中按固定顺序压入的寄存器/信息
typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // pusha 顺序
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

void (*interrupt_handlers[256])(registers_t* regs);

void isr_handler_c(registers_t* regs) {
    if (interrupt_handlers[regs->int_no]) {
        interrupt_handlers[regs->int_no](regs);
    } else {
        // 未注册：可以打印并停机或忽略
    }
}

void register_interrupt_handler(uint8_t n, void(*handler)(registers_t*)) {
    interrupt_handlers[n] = handler;
}
```

对于 IRQ，需要在处理完毕后向 PIC 发送 EOI（End Of Interrupt）：

```c
#define PIC_EOI 0x20
void send_eoi(unsigned char irq) {
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

// 在 C 的 IRQ handler 结尾调用 send_eoi(irq_no - 32);
```

## 设置 IDT 条目（示例实现）

```c
#define IDT_ENTRIES 256
struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr   idtp;

void set_idt_gate(int n, uint32_t handler_addr, uint16_t sel, uint8_t flags) {
    idt[n].base_lo = handler_addr & 0xFFFF;
    idt[n].sel = sel;
    idt[n].always0 = 0;
    idt[n].flags = flags;
    idt[n].base_hi = (handler_addr >> 16) & 0xFFFF;
}

extern void idt_load(); // 在汇编中实现 lidt [idtp]

void init_idt() {
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base  = (uint32_t)&idt;

    memset(&idt, 0, sizeof(struct idt_entry) * IDT_ENTRIES);

    // 安装异常/IRQ 的 gate（示例：将 0..31 的异常、32..47 的 IRQ）
    // set_idt_gate(vector, (uint32_t)handler_stub, KERNEL_CS, 0x8E);

    idt_load(); // 调用汇编函数执行 lidt
}
```

IDT 的加载通常在汇编中通过类似操作完成：

```nasm
; idt_load.s
extern idtp
global idt_load
idt_load:
    lea eax, [idtp]
    lidt [eax]
    ret
```

## 典型 IRQ 处理流程

1. 外设触发 IRQ，经 PIC/IOAPIC 发送到 CPU
2. CPU 使用中断向量索引 IDT，查找对应的中断门
3. 硬件自动切换到内核 CS/SS（如从用户态），抛出堆栈帧并跳转到 ISR stub
4. ISR stub 保存寄存器并调用 C 层通用处理器入口，C 层调用已注册的设备处理函数
5. 处理结束后发送 EOI 并从 iret 返回

## 调试与测试

- 在 QEMU 中测试：将内核镜像用 GRUB 启动，打印日志到 VGA 或串口（serial），并触发一个已知的 IRQ（例如计时器 IRQ0），验证 handler 被调用。
- 使用 bochs 或 QEMU 的 -serial stdio，把内核的串口输出映射到主机终端，便于查看 debug 信息。
- 对于异常（如除零），在 isr handler 打印寄存器状态，并在未处理时挂起，以便分析。

## 常见陷阱与注意事项

- 忘记重映射 PIC 会导致 IRQ 与 CPU 异常冲突（比如计时器触发被误判为异常）。
- stub 保存/恢复寄存器顺序要与 C 层 registers_t 定义一致。
- 在多核/IOAPIC 环境中，IOAPIC/Local APIC 的配置与 EOI 机制不同，需使用 APIC 写寄存器发送 EOI。
- 使用 amigable 的中断门属性（0x8E）通常适用于内核模式中断；需要用户态可访问的中断（例如系统调用）会使用不同 DPL。

## 参考

- OSDev Wiki — Interrupts: [https://wiki.osdev.org/Interrupts](https://wiki.osdev.org/Interrupts)
- OSDev Wiki — IDT: [https://wiki.osdev.org/Interrupt_Descriptor_Table](https://wiki.osdev.org/Interrupt_Descriptor_Table)
- OSDev Wiki — PIC: [https://wiki.osdev.org/8259_PIC](https://wiki.osdev.org/8259_PIC)

-- 完 --
