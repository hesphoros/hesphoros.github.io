# 操作系统启动流程简述（BIOS -> MBR -> GRUB）

本文简要概述基于传统 BIOS 的 x86 机器从上电到把控制权交给操作系统内核的典型启动流程，重点说明 MBR 与 GRUB 在其中的位置与作用，并给出调试/验证技巧与参考链接。

## 概览（高层次）

1. 上电自检（POST）与固件初始化
2. BIOS 加载并执行引导设备（通常先从第一个硬盘/软盘/光盘）上的主引导记录（MBR）的第一阶段代码（通常 446 字节）
3. MBR 代码寻找并加载“活动分区”的引导扇区或直接把控制权交给安装的引导管理器（如 GRUB stage1）
4. GRUB（或其他 bootloader）继续加载更复杂的阶段（stage1.5、stage2 或 core.img），解析配置并加载内核（以及模块），可根据需要遵循 Multiboot 协议传递信息给内核
5. 内核接管硬件，完成引导后的初始化并启动第一个用户空间进程

## 阶段细分与要点

### 1. BIOS / 固件

- 上电后进行 POST（Power-On Self Test），设置硬件并初始化中断向量、设备、内存检测等。
- BIOS 读取启动设备顺序（Boot Order），定位第一个可引导设备并读取其第一个扇区（LBA 0）的 512 字节到内存地址 0x7C00（传统）并跳转执行。
- 注意：现代 PC 可能使用 UEFI（并非本文描述），UEFI 的启动模型与 BIOS 有很大差异（EFI 可直接读取文件系统并加载 EFI 可执行文件）。

### 2. MBR（Master Boot Record）

- MBR 大小为 512 字节，其中：
  - 前 446 字节：引导程序代码（bootloader stage1 或跳转到更完整的加载器）
  - 接下来的 64 字节：分区表（4 个 16 字节的分区条目）
  - 最后 2 字节：MBR 签名 0x55AA
- MBR 的任务是找到活动分区或安装的二级引导加载器（例如 GRUB 的 stage1），并把控制权交给它。
- MBR 代码很短，通常只能加载下一个阶段（stage1.5 或 stage2 的一部分），因此需要多个阶段来完成复杂功能（如文件系统读取）。

### 3. GRUB 的分层（传统 BIOS 模式）

- GRUB 的经典分层（以 GRUB Legacy 与 GRUB2 的常见实现为例）：
  - stage1（位于 MBR 的前 446 字节或作为独立安装到引导扇区的代码）：极小，功能只用于加载下一阶段。
  - stage1.5（可选，位于紧跟 MBR 的空闲扇区或 core.img 中）：包含最少量的文件系统驱动以便读取文件系统中的 stage2/core.img。
  - stage2 / core.img：更完整的 GRUB 核心，支持读取配置文件（grub.cfg）、识别文件系统、显示菜单、加载模块与内核。
- GRUB2 通常把更复杂的代码放在 core.img 中（可安装在 BIOS 的 "post-MBR gap" 或文件系统中），并通过一个小的 stub 在 MBR 中跳转到 core.img。

### 4. GRUB 加载内核（与 Multiboot）

- GRUB 支持多种内核加载约定（直接加载 Linux zImage/ELF，或通过 Multiboot 协议加载自定义内核）。
- Multiboot 模式下：内核在其镜像中包含 Multiboot 头（magic/flags/checksum 等），GRUB 识别后会把内核加载到内存并在 EAX 中放入 Multiboot bootloader magic（0x2BADB002），在 EBX 中传递 multiboot_info 结构地址，然后跳转到内核入口。内核可检查 EAX 是否为该 magic 以确认由 Multiboot 引导。
- GRUB 在加载内核时通常会：
  - 解析配置（grub.cfg）并决定内核文件与命令行参数
  - 将内核映像读入内存（根据 ELF/Multiboot metadata 处理 load 地址或使用 Multiboot header 中的地址字段）
  - 加载 initrd/ramdisk (若配置)，并把其地址放入 multiboot_info 或传递到内核需要的位置
  - 设置必要的引导参数并跳转到内核入口

### 5. 内核接管

- 在内核入口处，内核会完成早期初始化（设置页表、中断、设备驱动、内存管理等），并最终启动用户空间 init 进程或进一步的引导流程。

## 关键数据结构与魔数

- MBR 签名：0x55AA（位于扇区末尾）
- Multiboot header magic（位于内核映像里）：0x1BADB002（用于在内核镜像中标识 Multiboot 头）
- Multiboot bootloader magic（bootloader 传递给内核的 EAX 值）：0x2BADB002（内核在入口时检查该值）

## 常见问题与调试技巧

- 如何验证某个二进制文件是否包含 Multiboot 头？
  - 在文件前 8192 字节内搜索 0x1BADB002（注意字节序为 little-endian）；并读取随后的 flags 与 checksum 做校验（magic+flags+checksum == 0 mod 2^32）。
- 如何从 GRUB 启动指定内核并传递命令行？
  - 在 grub.cfg 中使用 `linux`（或 `multiboot`）和 `initrd` 指令，或在 GRUB 菜单中按 `e` 编辑启动项。GRUB 会把命令行地址放入 multiboot_info（或以内核约定传递）。
- 使用 QEMU/VirtualBox 进行快速调试：
  - QEMU 常用选项：

```powershell
qemu-system-x86_64 -hda disk.img -m 512M -boot d -serial stdio
```

- 用 dd 制作测试镜像并写入 MBR：

```powershell
# 将 mbr.bin 写入磁盘镜像的前 512 字节
dd if=mbr.bin of=disk.img bs=512 count=1 conv=notrunc
```

- 在 GRUB 层可以使用 `ls`, `cat`, `set`, `insmod`, `linux`, `initrd`, `boot` 等命令来查看设备、加载内核并启动。

## 参考链接

- GNU GRUB Manual
- Multiboot Specification (GNU GRUB)
- OSDev Wiki：Booting

---

。