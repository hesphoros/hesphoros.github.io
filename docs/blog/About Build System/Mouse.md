# 鼠标支持（Mouse）在操作系统中的实现指南

本文件基于 OSDev Wiki 的相关条目，说明如何在 x86 操作系统中实现鼠标（以 PS/2 鼠标为主，兼顾 Intellimouse/滚轮扩展），内容包含硬件接口、初始化序列、数据包解析、IRQ 处理、事件队列、坐标处理与在 QEMU 中的测试方法。目标是提供可复制的步骤和示例代码。

> 说明：现代 USB 鼠标作为 HID 设备需要 USB 主机驱动（不在本文主线）。本文聚焦 PS/2 鼠标，它在虚拟机（QEMU/Bochs）中也能很方便地测试。

## 概览（高层步骤）

1. 确认已实现中断/IDT；PS/2 鼠标通常使用 IRQ12（重映射后对应 IDT 向量 44）。
2. 通过 PS/2 控制器启用辅助设备（鼠标），并向鼠标发送初始化命令（例如启用数据报告）。
3. 在 IDT 中为鼠标向量安装 ISR stub；ISR 从数据端口（0x60）读取字节并推入缓冲区。
4. 按鼠标协议解析收到的数据包（常见 3 字节或 4 字节包），生成坐标增量、按键状态、滚轮事件，并推送到上层事件队列或光标管理器。

## 硬件接口与端口

- PS/2 数据端口（键盘/鼠标共享）：0x60
- PS/2 命令/状态端口（控制器）：0x64

和键盘类似，读写需要检查控制器状态寄存器的相应位以判断缓冲区是否可读/可写。

## 鼠标初始化（PS/2 控制器层面）

初始化步骤（简化）：

1. 禁用鼠标中断或在安全上下文中操作。
2. 向控制器发送命令 0xA8：启用辅助设备（鼠标）。
3. 读取控制器配置字节（通过 0x20/0x60 的读写序列），设置合适位（例如启用 IRQ12）。
4. 向鼠标发送 0xF6（设置默认值）以及 0xF4（启用数据报告），并等待 ACK（0xFA）。可选：设置采样率（0xF3）和解析 Intellimouse（滚轮）协议。

示例伪代码：

```c
void ps2_wait_write() { while (inb(0x64) & 0x02); }
void ps2_wait_read()  { while (!(inb(0x64) & 0x01)); }

void mouse_write(uint8_t data) {
    ps2_wait_write();
    outb(0x64, 0xD4); // 告诉控制器下一字节是发给鼠标
    ps2_wait_write();
    outb(0x60, data);
}

uint8_t mouse_read_ack() {
    ps2_wait_read();
    return inb(0x60);
}

void mouse_init() {
    // 启用鼠标设备
    ps2_wait_write(); outb(0x64, 0xA8);
    // 设置默认设置
    mouse_write(0xF6); mouse_read_ack();
    // 启用数据报告
    mouse_write(0xF4); mouse_read_ack();
}
```

注意：实际驱动需要处理 ACK（0xFA）和可能的 NACK（0xFE），并在控制器忙时重试。

## PS/2 鼠标数据包格式（标准三字节）

标准 PS/2 鼠标在没有扩展（滚轮）的情况下发送 3 字节数据包：

- Byte 0: [Yovf Xovf Ysign Xsign 1 Middle Right Left]
- Byte 1: X movement (8-bit)
- Byte 2: Y movement (8-bit)

字段说明：

- Left/Right/Middle：按键状态位（1 = 按下）
- Xsign/Ysign：X/Y 的符号位（与 Byte1/2 的符号位一致）
- Xovf/Yovf：溢出位，表示移动量超出范围（应适当处理，通常丢弃或饱和）

Intellimouse（带滚轮）的扩展一般为 4 字节包，第四字节为滚轮增量（signed）。某些扩展还支持更多按钮。

## ISR 与数据收集示例

通常做法：鼠标 ISR（IRQ12）在中断上下文读出每个字节并将其放入缓冲队列，解析器从队列中按顺序组装完整数据包并处理。示例：

```c
#define MOUSE_BUF_SIZE 256
static uint8_t mouse_buf[MOUSE_BUF_SIZE];
static volatile unsigned int m_head=0, m_tail=0;

void mouse_push(uint8_t b){ unsigned int n=(m_head+1)%MOUSE_BUF_SIZE; if(n!=m_tail){ mouse_buf[m_head]=b; m_head=n; }}

void mouse_isr(registers_t *r){
    (void)r;
    uint8_t b = inb(0x60);
    mouse_push(b);
    // 向 PIC 发送 EOI（IRQ12）
    outb(0xA0, 0x20); // 从 PIC
    outb(0x20, 0x20); // 主 PIC
}

// 解析器（在任务或软中断中运行）
int mouse_get_byte(uint8_t *b){ if(m_tail==m_head) return 0; *b=mouse_buf[m_tail]; m_tail=(m_tail+1)%MOUSE_BUF_SIZE; return 1; }

void mouse_process_packets(){
    uint8_t p[4];
    while (mouse_get_byte(&p[0])){
        // 仅当第一个字节的位3（always 1）被置位时为同步包
        if (!(p[0] & 0x08)) continue; // 丢弃直到同步
        // 需要确保后续字节可用
        if (!mouse_get_byte(&p[1])) break;
        if (!mouse_get_byte(&p[2])) break;
        // 处理三字节包
        int x = (int8_t)p[1];
        int y = (int8_t)p[2];
        int left = p[0] & 0x01;
        int right = (p[0] & 0x02) >> 1;
        int middle = (p[0] & 0x04) >> 2;
        // y 通常是从底向上，可能需要取反依你坐标系
        // 将 x,y 累加到光标坐标并限制在屏幕范围
    }
}
```

注意：同步位（byte0 的 3 位）用于对齐数据包，以防中断期间丢包导致错位。

## 坐标管理与事件上报

- 驱动通常维护一个光标位置（x,y），并按收到的增量更新。需要对溢出位做防护处理。
- 将按键事件（按下/释放）以及移动/滚轮事件封装成统一的事件结构并推入系统事件队列或直接发给 GUI/终端层。

示例事件结构：

```c
typedef struct {
    int dx, dy; // 增量
    int wheel;  // 滚轮增量
    bool left, right, middle;
} mouse_event_t;
```

## Intellimouse（滚轮）自动识别

要识别 Intellimouse（3 字节 vs 4 字节包），常用方法是设置采样率序列：向鼠标发送三次 200, 100, 80（通过 0xF3 设置采样率），然后发送 0xF2（请求设备 ID），若返回 ID=3 则支持滚轮（或 ID=4 表示更多扩展）。详见 OSDev 的 Intellimouse 条目。

## 测试与调试（QEMU）

- 在 QEMU 启动并加载你的内核后，直接在虚拟机窗口中移动鼠标并观察串口/VGA 输出。
- QEMU monitor 提供 `sendmouse` 命令，可以从命令行发送模拟鼠标事件以便可重复测试。

示例 QEMU 启动命令（PowerShell）：

```powershell
qemu-system-i386 -kernel mykernel.bin -serial stdio -append "root=/dev/ram"
```

## USB 鼠标简要说明

如果目标平台使用 USB 鼠标，需要实现 USB host 控制器驱动并解析 HID 报告描述符；USB 鼠标通常通过中断或轮询传输提供报告，格式由 HID 定义。

## 常见陷阱

- 忽视同步位会导致数据包错位和错误解析。确保在解析器中使用同步位重新对齐。
- 忽视 ACK/NACK 或控制器忙位会导致初始化失败；实现重试与超时机制。
- 在 ISR 中执行重量级处理（例如复杂解析或 I/O）会降低实时性，应尽量把工作推迟到软中断或内核线程。

## 可落地文件清单（建议）

- `asm/isr_mouse.s`：鼠标 ISR stub（读取 0x60 并调用 mouse_isr）。
- `drivers/mouse.c`：初始化、缓冲、解析器、事件 API（mouse_init, mouse_poll_event, register_mouse_handler）。
- `include/mouse.h`：事件定义与公用 API。

## 参考

- [OSDev Wiki — PS/2 Mouse](https://wiki.osdev.org/PS/2_Mouse)
- [OSDev Wiki — PS/2 Controller](https://wiki.osdev.org/PS/2)
- [OSDev Wiki — Intellimouse](https://wiki.osdev.org/IntelliMouse)

-- 完 --
