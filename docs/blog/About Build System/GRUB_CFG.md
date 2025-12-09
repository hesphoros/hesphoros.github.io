# GRUB 配置（grub.cfg）指南

本文件说明如何编写和理解 GRUB2 使用的主配置文件 `grub.cfg`。内容以实用示例为主，覆盖 BIOS/UEFI 常见场景、Multiboot 与直接引导 ELF/vmlinuz、ISO loopback 与 chainloader 等。

> 说明：通常你不需要手写 `grub.cfg` —— 发行版通过 `grub-mkconfig` / `grub2-mkconfig` 或生成器自动生成。但手动编写/调试 `grub.cfg` 对学习引导过程和制作引导镜像很有帮助。

## 文件位置与加载

- 常见位置：`/boot/grub/grub.cfg`（或 `EFI/GRUB/grub.cfg` 在某些 UEFI 环境）。
- GRUB 在启动时会读取此文件（或由 core.img/EFI 指定）。当使用 Live ISO 或自制引导镜像时，`grub.cfg` 可放在可引导分区或 ISO 的 `/boot/grub` 下。

## 基本语法要点

- `set VARIABLE=value`：设置变量。
- `insmod <module>`：加载模块（`part_msdos`、`fat`、`ext2`、`loopback`、`gzio`、`iso9660` 等）。
- `search --fs-uuid <UUID> --set=root`：查找分区并设置 `root` 变量为找到的设备。
- `menuentry 'title' [--class foo] { ... }`：一个可选引导项。多用于交互式菜单。
- `multiboot` / `multiboot2`：用于直接加载 Multiboot 标准的内核（例如自写内核带 multiboot header）。
- `linux`, `linux16`, `linuxefi`：加载 Linux 内核（BIOS 环境通常使用 `linux`/`linux16`，UEFI 用 `linuxefi`）。
- `initrd`, `initrd16`, `initrdefi`：加载 initrd/initramfs。
- `chainloader +1`：链式加载另一个引导扇区（MBR/another bootloader）。

注：不同 GRUB 构建（BIOS vs UEFI）支持不同命令后缀（`linux` vs `linuxefi`）。

## 最小示例：用于 GRUB2 的简单 `grub.cfg`

```cfg
set timeout=3
set default=0

menuentry "My Linux" {
  set root=(hd0,1)
  linux /vmlinuz-5.15.0 root=/dev/sda1 ro quiet
  initrd /initramfs.img
}
```

解释：`set root=(hd0,1)` 表示第一个磁盘第一个分区（GRUB 的磁盘编号从 0 开始，分区在传统 BIOS 下从 1 开始）。在实际情况下，推荐使用 `search --fs-uuid` 或 `search --file` 来避免设备编号变化。

## 使用 `search` 更可靠地定位分区

```cfg
set timeout=3
menuentry "My Linux (by UUID)" {
  search --no-floppy --fs-uuid --set=root  0123-ABCD
  linux /vmlinuz root=/dev/disk/by-uuid/0123-ABCD ro quiet
  initrd /initramfs.img
}
```

或者使用 `search --file`（当你知道某个文件存在于分区上）

```cfg
search --no-floppy --file --set=root /vmlinuz
```

## 引导 Multiboot 内核（自制内核带 Multiboot 头）

如果你的内核符合 Multiboot（v0.6.96）规范，可以使用 `multiboot` 命令：

```cfg
menuentry "My Multiboot Kernel" {
  set root=(hd0,1)
  multiboot /kernel.bin root=/dev/sda1
  module /initrd.img
}
```

注意：`multiboot` 命令会直接调用 multiboot 规范方式启动，`module` 用于向内核传递模块（如 initrd）。有些旧版/特殊 GRUB 构建可能使用 `multiboot2`。

## UEFI 下的 Linux 引导（`linuxefi` / `initrdefi`）

UEFI 下通常使用 `linuxefi` 与 `initrdefi`：

```cfg
menuentry "Linux (UEFI)" {
  search --no-floppy --fs-uuid --set=root  0123-ABCD
  linuxefi /vmlinuz-5.15 root=UUID=0123-ABCD rw console=tty1
  initrdefi /initramfs.img
}
```

GRUB 会在 EFI 环境里调用内核的 EFI 启动路径。

## chainloader（链式加载）示例

如果你想加载 Windows 或另一块磁盘上的引导扇区：

```cfg
menuentry "Chainload Windows" {
  set root=(hd0,2)
  chainloader +1
}
```

对于 EFI 平台链式加载 `.efi` 文件：

```cfg
menuentry "Windows EFI" {
  search --fs-uuid --no-floppy --set=root ABCD-1234
  chainloader /EFI/Microsoft/Boot/bootmgfw.efi
}
```

## 从 ISO 启动（loopback）示例

常用于制作可引导 ISO（例如 live ISO）：

```cfg
insmod loopback
insmod iso9660
insmod gzio

menuentry "Boot ISO" {
  set isofile="/boot/iso/my.iso"
  loopback loop $isofile
  set root=(loop)
  linux /casper/vmlinuz boot=casper iso-scan/filename=$isofile quiet splash ---
  initrd /casper/initrd.lz
}
```

注意：不同发行版的 live ISO 需要不同的内核参数（如 `boot=casper`、`toram` 等）。

## 高级示例：带注释的完整 `grub.cfg`

```cfg
# /boot/grub/grub.cfg
set default=0
set timeout=5

# 搜索并设置 root（按 UUID）
search --no-floppy --fs-uuid --set=root  0123-ABCD

menuentry "My Linux (safe)" --class linux --class gnu-linux {
  echo "Loading Linux..."
  linux /vmlinuz root=UUID=0123-ABCD rw loglevel=3
  initrd /initramfs.img
}

submenu "Advanced options" {
  menuentry "My Linux (recovery)" {
    linux /vmlinuz root=UUID=0123-ABCD rw single
    initrd /initramfs.img
  }
}

menuentry "Boot other OS" {
  set root=(hd0,2)
  chainloader +1
}
```

## 常见选项与技巧

- `set timeout=0`：立即引导默认条目（无等待）。
- `savedefault` / `save_env`：配合 `GRUB_DEFAULT=saved` 使用，可以保存上次启动选择。
- 使用 `--class` 为 menuentry 添加 CSS/图标类（GRUB 主题相关）。
- 使用 `echo`, `pause`, `terminal_output` 做简单调试。
- `insmod normal` 与 `normal`：恢复 GRUB 的 normal 模式（在故障恢复中有用）。

## 自动生成 `grub.cfg`

在大多数 Linux 发行版中，不直接编辑 `grub.cfg`，而是编辑 `/etc/default/grub` 与 `/etc/grub.d/*`，然后运行：

```
sudo grub-mkconfig -o /boot/grub/grub.cfg
```

这条命令会合并模板并写出最终 `grub.cfg`。

## 调试与常见问题

- 看不到 `grub.cfg`：确认 GRUB 正在读取的分区（用 `insmod part_msdos` / `insmod part_gpt` + `ls` 在 GRUB 命令行中检查）。
- 设备编号变化：生产环境尽量使用 `search --fs-uuid` 或 `search --file`，不要硬编码 `(hd0,1)`。
- UEFI vs BIOS：在 UEFI 下，使用 `linuxefi`/`initrdefi`；在 BIOS 下使用 `linux`/`initrd`（或 `linux16`/`initrd16` 在旧系统中）。
- Multiboot 内核未启动：检查内核是否包含多引导头（magic/flags/checksum），并确认使用 `multiboot` 而非 `linux`。
- ISO loopback 失败：确保加载 `loopback`, `iso9660` 和 `gzio`（如需要），并使用正确的内核参数。

## 参考与延伸

- [GRUB manual](https://www.gnu.org/software/grub/manual/)
- 常见 `grub-mkconfig` 与 `/etc/grub.d` 模板用法：参见发行版文档（Debian/Ubuntu、Fedora 等）。

