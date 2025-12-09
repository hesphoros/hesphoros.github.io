[Multiboot Specification version 0.6.96](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
[Multiboot 规范-CSDN博客](https://blog.csdn.net/uframer/article/details/373900)

- 典型的 Multiboot 头部写法：
    - multiboot header 通常包含三项连续的 32-bit 值：magic、flags、checksum，其中 checksum 通常设置为使 magic+flags+checksum == 0（mod 2^32）。例如：  
        .long MAGIC_NUMBER  
        .long MULTIBOOT_FLAGS  
        .long -(MAGIC_NUMBER + MULTIBOOT_FLAGS)
    - 把 `MAGIC_NUMBER` 写入头部后，引导程序（如 GRUB）在加载时会识别并按 Multiboot 协议提供信息给内核。


#  Multiboot的头分布
Multiboot 头的分布必须如下表所示：

| 偏移 (从头部开始) | 类型  | 字段名 | 是否必需 | 说明 | 示例 |
|---:|:---:|:---|:---:|:---|:---|
| 0  | u32 | magic | 必需 | 标识 Multiboot 头的魔数，固定为 0x1BADB002。 | 0x1BADB002 |
| 4  | u32 | flags | 必需 | 功能/请求位字段。低 16 位为必须满足的位（bootloader 必须理解或失败），高 16 位为可选位（bootloader 可忽略）。各位含义参见规范（如 bit0=页对齐，bit1=请求内存信息，bit2=请求视频模式，bit16=使用地址字段等）。 | e.g. 0x00010007 |
| 8  | u32 | checksum | 必需 | 与 magic 和 flags 相加的 32 位无符号和须为 0（magic + flags + checksum == 0 mod 2^32）。常用写法为 -(magic + flags)。 | -(MAGIC + FLAGS) |
| 12 | u32 | header_addr | 可选 (若 flags[16] = 1) | Multiboot 头被加载到物理内存中的起始地址，用于在非 ELF 可用时同步文件偏移到物理地址。 | multiboot_header (physical) |
| 16 | u32 | load_addr | 可选 (若 flags[16] = 1) | 文本（text）段在物理内存中的起始地址。load_addr <= header_addr。 | _start 的物理地址 |
| 20 | u32 | load_end_addr | 可选 (若 flags[16] = 1) | 数据段（text+data）在物理内存中的结束地址（用于计算需要加载的文件部分）。若为 0，表示加载整个文件。 | _edata 的物理地址 |
| 24 | u32 | bss_end_addr | 可选 (若 flags[16] = 1) | BSS 段（未初始化数据）结束的物理地址。引导程序需将该区域清零并保留。若为 0 表示无 bss。 | _end 的物理地址 |
| 28 | u32 | entry_addr | 可选 (若 flags[16] = 1) | 引导程序在加载完成后应跳转到的物理地址（内核入口）。 | multiboot_entry 的物理地址 |
| 32 | u32 | mode_type | 可选 (若 flags[2] = 1) | 指定首选显示模式：0=线性图形，1=EGA 文本，其他保留。引导程序可忽略。 | 0 或 1 |
| 36 | u32 | width | 可选 (若 flags[2] = 1) | 图形模式下的像素宽度，或文本模式下的列数；0 表示无偏好。 | 1024 |
| 40 | u32 | height | 可选 (若 flags[2] = 1) | 图形模式下的像素高度，或文本模式下的行数；0 表示无偏好。 | 768 |
| 44 | u32 | depth | 可选 (若 flags[2] = 1) | 图形模式下的像素深度（bits per pixel）；文本模式下为 0。0 表示无偏好。 | 32 |

备注：

- Multiboot 头必须完全位于 OS 映像的前 8192 字节内，并且以 32 位长字对齐（longword aligned）。
- 三个必需字段（magic、flags、checksum）必须存在且相邻；checksum 的计算规则为 32 位无符号求和为 0。常见汇编写法：

    .long MULTIBOOT_HEADER_MAGIC
    .long MULTIBOOT_HEADER_FLAGS
    .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

- 若设置了 flags 的第 16 位（MULTIBOOT_AOUT_KLUDGE），则 header_addr/load_addr/load_end_addr/bss_end_addr/entry_addr 字段有效，且字段中的地址均为物理地址。
- 若设置了 flags 的第 2 位（MULTIBOOT_VIDEO_MODE），则 mode_type/width/height/depth 字段有效，用于向引导程序建议首选视频模式（引导程序可选择其他模式）。

更多细节请参见 Multiboot 规范：Header magic fields、Header address fields 与 Header graphics fields。

## Checksum 详解与示例

Checksum 字段是第三个 32-bit 字段（在 magic 与 flags 之后），其目的是让三者的 32 位无符号和为 0。用数学式子表示：

```text
(magic + flags + checksum) mod 2^32 == 0
```

因此 checksum 的计算公式是：

```text
checksum = - (magic + flags)  (按 32-bit 无符号数，即模 2^32)
```

说明要点：

- 这是一个无符号 32 位的求和校验（即 32-bit 两补数下的取反加一结果）。
- 汇编器中常见且可靠的写法是直接使用宏或表达式：

```asm
.long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
```

  这样汇编器会在汇编时计算并把对应的 32-bit 值放入文件。
- 在计算时应当当心整型溢出：按照规范，使用模 2^32 的运算；换言之，算术是无符号的并会自然回绕。

数值示例（演示计算过程）：

```text
取例：

magic = 0x1BADB002
flags = 0x00010007   (例如 MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_MODE | MULTIBOOT_AOUT_KLUDGE)

先求和（按 32-bit）：

sum = magic + flags = 0x1BADB002 + 0x00010007 = 0x1BAEB009

取 checksum 为 sum 的 32-bit 二补数（或模 2^32 的相反数）：

checksum = -sum mod 2^32 = 0xE4514FF7

验证：

magic + flags + checksum = 0x1BADB002 + 0x00010007 + 0xE4514FF7 = 0x100000000 -> 0 (mod 2^32)
```

汇编示例（简洁写法，汇编器计算常量）：

```asm
.align 4
.long 0x1BADB002
.long 0x00010007
.long -(0x1BADB002 + 0x00010007)   /* 汇编器会计算出 0xE4514FF7 */
```

C 宏示例（在头文件中）：

```c
#define MULTIBOOT_HEADER_MAGIC 0x1BADB002U
#define MULTIBOOT_HEADER_FLAGS 0x00010007U
#define MULTIBOOT_HEADER_CHECKSUM (-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS))
```

运行/调试时的验证建议：

```c
if ((magic + flags + checksum) != 0) {
    /* 非 Multiboot 或头部损坏：报告并中止 */
}
```

常见陷阱与注意事项：

- 不要把这个 checksum 与 bootloader 向内核传递的 EAX magic（0x2BADB002）混淆，二者用途不同。前者用于在文件中识别 Multiboot 头并保证头内部一致性；后者由引导程序传递到内核，表明内核是由 Multiboot 引导的。
- 汇编器的表达式 `-(MAGIC + FLAGS)` 通常能正确生成 32-bit 的补码常数；但如果你用不同的工具链或非常古老的汇编器，建议先用本地脚本/程序计算并把常数写入以避免符号解析差异。
- 当 runtime 或工具将这些常数视为有符号打印时，可能会看到负数表示法，但实际存储/比较应基于无符号 32-bit 运算。

## Flags 位说明与示例

下面列出 Multiboot 头中常用的 `flags` 位（位号从 0 开始），包含位掩码、常见宏名与简短说明：

| 位号 (bit) | 掩码 (hex) | 宏名 (示例) | 含义 |
|:--:|:--:|:--|:--|
| 0  | 0x00000001 | MULTIBOOT_PAGE_ALIGN | 要求引导加载的模块按 4KB 页对齐（某些内核需要）。 |
| 1  | 0x00000002 | MULTIBOOT_MEMORY_INFO | 要求引导器提供基本内存信息（mem_lower, mem_upper）。 |
| 2  | 0x00000004 | MULTIBOOT_VIDEO_MODE | 要求提供视频/图形模式信息（mode_type/width/height/depth）。 |
| 3  | 0x00000008 | MULTIBOOT_MODS_PRESENT | 指示引导器应提供模块列表（mods_count, mods_addr）。 |
| 4  | 0x00000010 | MULTIBOOT_AOUT_SYMS | 表示 a.out 符号表信息在 Multiboot 信息结构中可用（与 bit5 互斥）。 |
| 5  | 0x00000020 | MULTIBOOT_ELF_SHDR | 表示 ELF 节头表信息在 Multiboot 信息结构中可用（与 bit4 互斥）。 |
| 6  | 0x00000040 | MULTIBOOT_MEM_MAP | 请求引导器提供完整内存映射（mmap_*）。 |
| 7  | 0x00000080 | MULTIBOOT_DRIVE_INFO | 请求驱动器信息（drives_*）。 |
| 8  | 0x00000100 | MULTIBOOT_CONFIG_TABLE | 请求 ROM 配置表地址（config_table）。 |
| 9  | 0x00000200 | MULTIBOOT_BOOT_LOADER_NAME | 请求引导器名称字符串（boot_loader_name）。 |
| 10 | 0x00000400 | MULTIBOOT_APM_TABLE | 请求 APM 表地址（apm_table）。 |
| 11 | 0x00000800 | MULTIBOOT_VBE_INFO | 请求 VBE 信息（vbe_control_info, vbe_mode_info, vbe_mode 等）。 |
| 12 | 0x00001000 | MULTIBOOT_FRAMEBUFFER_INFO | 请求帧缓冲信息（framebuffer_* 字段）。 |
| 16 | 0x00010000 | MULTIBOOT_AOUT_KLUDGE | 启用地址字段（header_addr, load_addr, ...），通常用于非 ELF 可执行格式。 |

常见的 flags 组合示例：

常见宏定义（来自 multiboot.h 示例）：

```c
#define MULTIBOOT_HEADER_FLAGS (MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_MODE | MULTIBOOT_AOUT_KLUDGE)
```

对应的十六进制值举例（按上面位）：

```text
MULTIBOOT_PAGE_ALIGN = 0x00000001
MULTIBOOT_MEMORY_INFO = 0x00000002
MULTIBOOT_VIDEO_MODE  = 0x00000004
MULTIBOOT_AOUT_KLUDGE = 0x00010000

组合值 = 0x00010007
```

如果只需要内存信息与模块列表：

```c
flags = MULTIBOOT_MEMORY_INFO | MULTIBOOT_MODS_PRESENT  /* -> 0x0000000A */
```

汇编/宏示例（常见写法）：

```asm
/* multiboot header */
.align 4
multiboot_header:
    .long MULTIBOOT_HEADER_MAGIC         /* magic */
    .long MULTIBOOT_HEADER_FLAGS         /* flags */
    .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS) /* checksum */

/* 当使用 a.out kludge 时还会追加地址字段（物理地址） */
    .long multiboot_header                /* header_addr */
    .long _start                          /* load_addr */
    .long _edata                          /* load_end_addr */
    .long _end                            /* bss_end_addr */
    .long multiboot_entry                 /* entry_addr */
```

在 C 代码中，你也可以使用 multiboot.h 中的宏来定义：

```c
#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
#define MULTIBOOT_HEADER_FLAGS (MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_MODE | MULTIBOOT_AOUT_KLUDGE)
#define MULTIBOOT_HEADER_CHECKSUM (-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS))
```
注意与边界条件：

- flags 中未定义的位必须为 0（以便未来扩展能够向后兼容）。
- bits 4 和 5（a.out 符号表与 ELF 节头表）互斥，不能同时设置。
- 当使用较新的显存/帧缓冲字段（bit12）时，framebuffer_addr 为 64-bit，但为兼容仍建议引导器尽量提供在 4GiB 以下的地址。

