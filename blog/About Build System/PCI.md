# PCI 总线枚举与配置（在 C 语言中实现）

本文档说明如何在操作系统内核中用 C 语言实现 PCI（Peripheral Component Interconnect）总线的枚举与设备配置。内容包含 PCI 配置空间访问（I/O 端口方法与 ECAM）、总线/设备/功能扫描、Vendor/Device ID 读取、BAR（Base Address Register）解析、中断引脚与 IRQ 路由、MSI/MSI-X 简介、以及在 QEMU/VirtualBox 中的测试方法与示例代码。

> 注：本文聚焦传统 PCI 与 PCI Express（PCIe）的配置空间访问基础，不涉及高级 PCIe 特性（例如 AER、热插拔、多功能设备的细节）。若需要支持更多设备类型或使用 ACPI 解析 MCFG 表，可在本文基础上扩展。

## PCI 概述

- **PCI 总线**：一种用于连接外设（网卡、声卡、GPU、存储控制器等）的标准总线。
- **配置空间**：每个 PCI 设备有一个 256 字节（PCI 2.x）或 4KB（PCIe）的配置空间，用于报告设备信息、配置 BAR、中断等。
- **寻址结构**：PCI 设备通过三元组 (Bus, Device, Function) 唯一标识（简称 BDF）。总线号 0-255，设备号 0-31，功能号 0-7。

## 配置空间访问方法

### 方法 1：I/O 端口（0xCF8 / 0xCFC）

传统 x86 系统使用两个 I/O 端口访问 PCI 配置空间：

- **0xCF8（CONFIG_ADDRESS）**：写入要访问的设备/寄存器地址（32 位）。
- **0xCFC（CONFIG_DATA）**：读写配置寄存器的数据（32 位）。

CONFIG_ADDRESS 格式（32 位）：

```text
 31       | 30-24   | 23-16 | 15-11  | 10-8     | 7-2       | 1-0
 Enable   | Reserved| Bus   | Device | Function | Register  | 00
 (1=enable)           (8 bit) (5 bit)  (3 bit)   (6 bit)
```

示例：读取配置寄存器的 C 函数：

```c
#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t val;
    __asm__ volatile ("inl %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

uint32_t pci_config_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = (uint32_t)(
        (1UL << 31) |           // enable bit
        ((uint32_t)bus << 16) |
        ((uint32_t)device << 11) |
        ((uint32_t)function << 8) |
        (offset & 0xFC)         // 对齐到 4 字节
    );
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void pci_config_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t val) {
    uint32_t address = (uint32_t)(
        (1UL << 31) | ((uint32_t)bus << 16) | ((uint32_t)device << 11) |
        ((uint32_t)function << 8) | (offset & 0xFC)
    );
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, val);
}
```

### 方法 2：ECAM（增强配置访问机制，PCIe）

PCIe 使用内存映射（MMIO）访问配置空间（通过 ACPI MCFG 表获取基址）。每个设备的配置空间为 4KB。地址计算：

```text
address = ecam_base + (bus << 20) | (device << 15) | (function << 12) | offset
```

示例（伪代码）：

```c
uint32_t pcie_config_read(uintptr_t ecam_base, uint8_t bus, uint8_t dev, uint8_t func, uint16_t offset) {
    volatile uint32_t *addr = (volatile uint32_t*)(
        ecam_base + ((bus << 20) | (dev << 15) | (func << 12) | offset)
    );
    return *addr;
}
```

本文后续主要使用 I/O 端口方法（0xCF8/0xCFC），ECAM 方法需额外解析 ACPI MCFG。

## 配置空间布局（PCI 2.x 标准头）

配置空间前 64 字节为标准头（Type 0 / Type 1），常用偏移：

| 偏移 | 大小 | 字段名            | 说明                              |
|------|------|-------------------|-----------------------------------|
| 0x00 | 2    | Vendor ID         | 厂商 ID（0xFFFF 表示设备不存在）  |
| 0x02 | 2    | Device ID         | 设备 ID                           |
| 0x04 | 2    | Command           | 命令寄存器（启用 I/O、MMIO、总线主控） |
| 0x06 | 2    | Status            | 状态寄存器                        |
| 0x08 | 1    | Revision ID       | 修订号                            |
| 0x09 | 1    | Prog IF           | 编程接口                          |
| 0x0A | 1    | Subclass          | 子类                              |
| 0x0B | 1    | Class Code        | 类代码                            |
| 0x0E | 1    | Header Type       | 头类型（0=普通设备，1=PCI桥）     |
| 0x10-0x27 | 6x4 | BAR 0-5        | 基地址寄存器（MMIO 或 I/O 端口）  |
| 0x3C | 1    | Interrupt Line    | 中断线（IRQ）                     |
| 0x3D | 1    | Interrupt Pin     | 中断引脚（1=INTA#, 2=INTB#, ...） |

## 总线/设备/功能扫描（枚举设备）

常用策略：暴力枚举所有可能的 (bus, device, function)，读取 Vendor ID 判断设备是否存在。

```c
void pci_scan_all() {
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint32_t val = pci_config_read(bus, device, function, 0x00);
                uint16_t vendor = val & 0xFFFF;
                uint16_t devid = (val >> 16) & 0xFFFF;
                if (vendor == 0xFFFF) {
                    // 设备不存在
                    if (function == 0) break; // 跳过该设备剩余功能
                    continue;
                }
                // 读取类代码
                uint32_t class_rev = pci_config_read(bus, device, function, 0x08);
                uint8_t class_code = (class_rev >> 24) & 0xFF;
                uint8_t subclass = (class_rev >> 16) & 0xFF;
                
                kprintf("[PCI] %02x:%02x.%x: %04x:%04x (class %02x:%02x)\n",
                        bus, device, function, vendor, devid, class_code, subclass);
                
                // 可选：解析 BAR、中断等
            }
        }
    }
}
```

优化：检查 Header Type 的多功能位（offset 0x0E，bit 7），若为 0 则仅扫描 function 0。

## BAR（基地址寄存器）解析

BAR 用于指定设备的 MMIO 或 I/O 端口地址范围。每个 BAR 是 32 位寄存器（可能为 64 位，使用两个连续的 BAR）。

### 读取 BAR 并确定大小

1. 读取当前 BAR 值（`bar_val = pci_config_read(..., bar_offset)`）。
2. 写全 1 到 BAR（`pci_config_write(..., bar_offset, 0xFFFFFFFF)`）。
3. 再次读取 BAR 得到掩码（`mask = pci_config_read(..., bar_offset)`）。
4. 恢复原始值（`pci_config_write(..., bar_offset, bar_val)`）。
5. 计算大小：`size = ~(mask & ~0xF) + 1`（对于 MMIO）或 `~(mask & ~0x3) + 1`（对于 I/O）。

BAR 位 0 用于区分 MMIO（0）或 I/O（1）。对于 MMIO，位 1-2 指示类型（0=32 位，2=64 位）。

示例：

```c
void pci_read_bar(uint8_t bus, uint8_t dev, uint8_t func, uint8_t bar_idx) {
    uint8_t offset = 0x10 + (bar_idx * 4);
    uint32_t bar = pci_config_read(bus, dev, func, offset);
    
    if (bar & 0x1) {
        // I/O 端口
        uint32_t base = bar & ~0x3;
        kprintf("  BAR%d: I/O Port 0x%x\n", bar_idx, base);
    } else {
        // MMIO
        uint8_t type = (bar >> 1) & 0x3;
        uint32_t base = bar & ~0xF;
        if (type == 2) {
            // 64 位 BAR，读取下一个 BAR 高 32 位
            uint32_t bar_hi = pci_config_read(bus, dev, func, offset + 4);
            uint64_t base64 = ((uint64_t)bar_hi << 32) | base;
            kprintf("  BAR%d: MMIO64 0x%llx\n", bar_idx, base64);
        } else {
            kprintf("  BAR%d: MMIO 0x%x\n", bar_idx, base);
        }
    }
}
```

## 中断配置

### 传统 INTx（INT A/B/C/D）

- **Interrupt Pin**（offset 0x3D）：1=INTA#, 2=INTB#, 3=INTC#, 4=INTD#, 0=无中断。
- **Interrupt Line**（offset 0x3C）：BIOS/OS 写入的 IRQ 号（例如 11），需根据平台的 APIC/IOAPIC 或 PIC 重映射。

读取示例：

```c
uint8_t int_line = pci_config_read(bus, dev, func, 0x3C) & 0xFF;
uint8_t int_pin = pci_config_read(bus, dev, func, 0x3D) & 0xFF;
if (int_pin != 0) {
    kprintf("  Interrupt: line=%d, pin=%d\n", int_line, int_pin);
}
```

### MSI / MSI-X（消息信号中断）

MSI 是 PCIe 的高级特性，允许设备直接向 APIC 发送消息而无需传统中断线。需要解析 Capability 链表（offset 0x34）并启用 MSI/MSI-X Capability。详见 PCI 规范与 OSDev MSI 条目。

## 启用总线主控（Bus Mastering）

若设备需要使用 DMA，需在 Command 寄存器（offset 0x04）设置 bit 2（Bus Master Enable）：

```c
void pci_enable_bus_master(uint8_t bus, uint8_t dev, uint8_t func) {
    uint32_t cmd = pci_config_read(bus, dev, func, 0x04);
    cmd |= (1 << 2);
    pci_config_write(bus, dev, func, 0x04, cmd);
}
```

## 测试与调试

### QEMU 中的 PCI 设备

QEMU 默认模拟多个 PCI 设备（例如主机桥、IDE 控制器、网卡等）。启动内核后扫描 PCI 总线，应能枚举出多个设备。例如：

```powershell
qemu-system-i386 -kernel mykernel.bin -serial stdio -net nic,model=e1000
```

上述命令添加了一个 Intel e1000 网卡（Vendor=0x8086, Device=0x100E），可在扫描中看到。

### VirtualBox 中的 PCI 设备

VirtualBox 也提供 PCI 设备模拟（例如 PIIX3 IDE、网卡等）。在 VM 设置中启用不同网络/存储适配器可增加可见的 PCI 设备。

### 调试输出示例

在扫描时打印发现的设备及其 BAR、中断信息，以验证枚举逻辑正确：

```text
[PCI] 00:00.0: 8086:1237 (class 06:00)  // 主机桥
[PCI] 00:01.0: 8086:7000 (class 06:01)  // ISA 桥
[PCI] 00:02.0: 8086:100E (class 02:00)  // 网卡
  BAR0: MMIO 0xFEBC0000
  Interrupt: line=11, pin=1
```

## 常见陷阱与注意事项

- 读取不存在设备的配置空间会返回 0xFFFFFFFF（Vendor ID = 0xFFFF）；需判断并跳过。
- 写 BAR 时确保设备未启用（Command 寄存器的 I/O/MMIO Enable 位应先清零），否则可能导致设备响应错误地址。
- BAR 大小计算需谨慎处理 64 位 BAR 和对齐要求。
- 中断路由与 APIC/IOAPIC 配置相关；Interrupt Line 值需与系统中断配置匹配。

## 可落地文件清单（建议）

- `drivers/pci.c`：PCI 扫描、配置读写、BAR 解析、中断查询。
- `include/pci.h`：PCI 常量定义（Vendor/Device ID 列表）、API 函数声明。
- `kernel/main.c`：在内核初始化时调用 `pci_scan_all()` 枚举设备并注册驱动。

## 参考资料

- [OSDev Wiki — PCI](https://wiki.osdev.org/PCI)
- [OSDev Wiki — PCI Express](https://wiki.osdev.org/PCI_Express)
- [PCI Local Bus Specification (PCI-SIG)](https://pcisig.com/specifications)

-- 完 --
