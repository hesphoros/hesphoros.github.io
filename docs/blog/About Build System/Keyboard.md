# 键盘驱动与在操作系统中实现键盘输入

本文档讲解如何在 x86 操作系统中实现键盘支持（以 PS/2 键盘为例），包含硬件、扫描码、IRQ/ISR、驱动架构、键映射、LED 控制与测试方法。目标是给出可复制的步骤和示例代码，帮助你把键盘输入接入终端/TTY 层。

> 备注：现代系统常用 USB 键盘（HID），那需要实现 USB 主机堆栈或使用现有驱动；本文聚焦传统 PS/2（也适用于 QEMU 虚拟键盘测试）。

## 概览（高层步骤）

1. 确保已实现并启用了中断/IDT（IRQ0..IRQ15 已安装且 PIC 已重映射）。键盘通常使用 IRQ1（经重映射后为向量 33）。
2. 在 IDT 中为键盘向量安装 ISR stub，ISR 中读取扫描码端口（0x60），并把扫描码交给内核的键盘处理函数。
3. 键盘处理函数将扫描码转换为键事件（按下/释放）、处理修饰键（Shift/Ctrl/Alt/CapsLock）、并把字符或按键事件推入环形缓冲区或事件队列。
4. 用户态或内核终端从缓冲区读取字符并显示/处理。

## 硬件接口（PS/2 基本端口）

- 数据端口：0x60（读取扫描码，向键盘发送命令/参数）
- 状态/命令端口：0x64（读取控制器状态，发送控制器命令）

常用 I/O 范例（C 内联或外部实现）：

```c
static inline void outb(uint16_t port, uint8_t val);
static inline uint8_t inb(uint16_t port);
```

具体实现依照你的编译器/平台（GCC 内联 asm 或单独的汇编函数）。

## 扫描码（Scancode）与常见集

- 扫描码是硬件发送的原始码，常见的有 Scancode Set 1、Set 2、Set 3。PC/AT 时代常用 Set 1 或 Set 2；QEMU/PC BIOS 多为 Set 1（兼容性好）。
- 在 Set 1 中，按下（make）和释放（break）码可以区分：释放通常是 make | 0x80（即高位设置）。例如，按下键产生 0x1E，释放产生 0x9E。
- 扩展键（例如右侧 Alt/Ctrl、方向键、部分功能键）会以 0xE0 前缀开头，接着一或多个字节。

实现上可以：先按字节接收扫描码，根据是否是 0xE0 前缀进入扩展状态，再根据最后得到的字节解析事件。

## 基本 ISR 与读取扫描码（示例）

假定 IDT 已为向量 33 安装 stub，下面是 C 层的简化处理流程：

```c
// 简化 registers_t，只演示关键字段
typedef struct { uint32_t int_no; /* ... */ } registers_t;

// 环形缓冲区
#define KBD_BUF_SIZE 256
static uint8_t kbd_buf[KBD_BUF_SIZE];
static volatile unsigned int kbd_head = 0, kbd_tail = 0;

void keyboard_push(uint8_t scancode) {
    unsigned int next = (kbd_head + 1) % KBD_BUF_SIZE;
    if (next != kbd_tail) { // 非满
        kbd_buf[kbd_head] = scancode;
        kbd_head = next;
    }
}

void keyboard_isr_handler(registers_t *regs) {
    (void)regs;
    uint8_t sc = inb(0x60);
    keyboard_push(sc);
    // 发送 EOI 给 PIC（假设 IRQ1）
    outb(0x20, 0x20);
}

// 轮询读取（例如恢复场景）
int keyboard_get_scancode_poll(uint8_t *out) {
    uint8_t status = inb(0x64);
    if (status & 0x01) { // 输出缓冲区满
        *out = inb(0x60);
        return 1;
    }
    return 0;
}
```

注意：ISR 中应尽量短小，不做阻塞操作，建议仅读取扫描码并把最少量的数据推入缓冲区/队列，然后尽快返回。

## 扫描码到按键/字符的映射

最简单的实现使用两张表：未按 Shift 的映射表和按 Shift 的映射表（ASCII 或按键码）。例如：

```c
static const char keymap_unshift[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t', 'q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, /* control */ 'a','s','d','f','g','h','j','k','l',';','\'','`',
    0, /* left shift */ '\','z','x','c','v','b','n','m',',','.','/',
    0, /* right shift */ '*', 0, /* alt */ ' ', /* space */
    /* ... */
};

static const char keymap_shift[128] = {
    0,  27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t', 'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0, /* control */ 'A','S','D','F','G','H','J','K','L',':','"','~',
    0, /* left shift */ '|','Z','X','C','V','B','N','M','<','>','?',
    0, /* right shift */ '*', 0, /* alt */ ' ', /* space */
    /* ... */
};
```

解析逻辑举例（伪代码）：

1. 从环形缓冲区取出一个扫描码字节 sc。
2. 如果 sc == 0xE0，设置扩展标志并继续读取下一字节以解析扩展键。
3. 如果 sc & 0x80：这是释放（break）码 -> 根据低 7 位查找对应按键并清除修饰键状态（如 Shift 释放）。
4. 否则这是按下（make）码 -> 根据当前修饰键状态（Shift/CapsLock）选择 keymap，得到字符；把字符或键事件推入上层（TTY/事件队列）。

注意：不同扫描码集与扩展序列略有差异，建议以目标环境（QEMU/BIOS）为准进行简单测试并打印收到的扫描码序列以验证。

## 修饰键与锁定键处理

- 记录 Shift、Ctrl、Alt 的按下/释放状态（布尔量）。
- CapsLock、NumLock、ScrollLock 通常在按下对应键时切换状态并通过命令更新键盘 LED（见 LED 控制）。

## LED 控制（发送命令到键盘）

键盘 LED 通过向键盘发送命令 0xED 然后发送一个字节位掩码（位0=Scroll, 位1=Num, 位2=Caps）。示例：

```c
// 发送命令需要检查控制器是否可写（检查 0x64 的位 1 = 输入缓冲区空）
void wait_write() {
    while (inb(0x64) & 0x02) ;
}

void kb_set_leds(uint8_t ledmask) {
    wait_write();
    outb(0x60, 0xED); // 请求设置 LED
    wait_write();
    outb(0x60, ledmask);
}
```

注意：键盘在接收命令/数据时会返回 ACK（0xFA）或不 ACK，驱动可选择轮询 ACK 或简单重试。

## 驱动设计建议（体系结构）

- 硬件层（ISR）：尽量短，仅读取扫描码并入队。
- 翻译层（键盘任务/内核线程）：负责合并多字节扫描码、处理扩展码、维护修饰键、产生命令/字符，并把最终事件推给终端层。
- 终端/TTY 层：实现行纪律、回显、编辑键、历史等功能。

使用单独的内核工作队列或软中断（softirq/tasklet）来在中断上下文外完成较重的工作，例如字符编码与回显。

## 多核与并发注意事项

- 环形缓冲区的读写需要原子操作或在禁用中断/使用自旋锁保护期间进行（视实现）。
- 在 SMP 系统中，IRQ 可能被路由到特定 CPU，考虑把键盘事件统一送到负责输入/终端的 CPU 或用无锁队列。

## QEMU/测试方法

- 在 QEMU 中启动内核并把串口/终端输出连接到主机：

```powershell
# 在 Windows PowerShell 环境下示例（你的镜像名可能不同）
qemu-system-i386 -kernel mykernel.bin -serial stdio -append "root=/dev/ram"
```

- 在 QEMU 里可以直接按键测试或使用 `sendkey` 命令通过 QEMU monitor 发送按键序列。
- 建议在 ISR 中打印（串口或 VGA）收到的扫描码进行调试，确认是你预期的 Scancode Set（随后再映射成字符）。

## USB 键盘（简要）

若目标平台使用 USB 键盘，需实现 USB 主机控制器驱动（UHCI/OHCI/EHCI/XHCI）和 HID 类解析器，或借助现成的 USB 堆栈。USB 键盘不会走 IRQ1/0x60 机制，而是通过主机控制器中断/轮询传输交付报告。

## 示例清单（可落地为文件）

- `asm/isr_keyboard.s`：键盘 ISR stub（push regs, call keyboard_isr_handler, iret）。
- `drivers/keyboard.c`：环形缓冲、scan->key 解析、register handler 接口、LED 控制函数。
- `include/keyboard.h`：事件结构、API：kbd_init(), kbd_getchar(), register_key_event_handler() 等。

如果你希望我把示例代码真正加入仓库并在 QEMU 中做一次快速运行验证，我可以：

1) 在 `drivers/keyboard.c` 和 `asm/isr_keyboard.s` 中落地上面示例；
2) 修改或补充简单的内核主循环以在串口打印按键事件；
3) 在 PowerShell 中调用 QEMU 启动并收集输出；

请告诉我你更希望我：

- 仅生成文档（到此完成），还是
- 同时把示例代码放入仓库并做一次 QEMU 验证（我将同时创建/编辑代码文件并运行一轮快速测试）。

## 参考资料

- [OSDev Wiki — Keyboard](https://wiki.osdev.org/Keyboard)
- [OSDev Wiki — PS/2 Controller](https://wiki.osdev.org/PS/2)
- [OSDev Wiki — Scancode Set](https://wiki.osdev.org/Scancode_Set)

-- 完 --
