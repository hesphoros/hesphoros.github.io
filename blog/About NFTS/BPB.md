![](../picture/Pasted%20image%2020260106185150.png)# BIOS Parameter Block

# 什么是BPB

**BPB（BIOS Parameter Block）**
 是 **FAT 文件系统**（FAT12 / FAT16 / FAT32）中，用来**描述磁盘/分区布局的结构体**。

。磁盘大小和几何变量封装在 BPB 中

> 操作系统通过 BPB 才知道：
>
> - 一个扇区多大
> - 一个簇包含多少扇区
> - FAT 表在哪
> - 根目录在哪

# BPB 在磁盘上的位置

```
┌──────────────────────────────┐
│ Boot Sector（引导扇区）        │  ← 第 0 扇区
│ ├─ Jump 指令 (3B)             │
│ ├─ OEM Name (8B)             │
│ ├─ BPB（核心参数）            │ ← 就在这里
│ ├─ 扩展 BPB                  │
│ ├─ 引导代码                   │
│ └─ 0x55AA 结束标志            │
└──────────────────────────────┘

```

 **永远在分区的第一个扇区（LBA 0 / 相对分区起始）**

在 NTFS 文件系统中，**BPB（BIOS Parameter Block，BIOS 参数块）** 位于分区的第一个扇区（即引导扇区 Boot Sector）中。它的作用是向操作系统和引导程序（如 `BOOTMGR`）提供分区的物理布局信息，以便正确定位文件系统数据。

NTFS 的引导扇区通常跨越 16 个扇区，但核心的 BPB 结构位于第一个扇区的偏移 **0x0B** 处。

#  NTFS 引导扇区概览

在深入 BPB 之前，先看引导扇区的整体结构：

| **偏移 (Hex)** | **长度 (Bytes)** | **描述**                                       |
| -------------- | ---------------- | ---------------------------------------------- |
| 0x00           | 3                | 跳转指令 (Jump Instruction)，通常为 `EB 52 90` |
| 0x03           | 8                | OEM ID，固定为字符串 `"NTFS    "`              |
| **0x0B**       | **25**           | **标准 BPB**                                   |
| **0x24**       | **48**           | **扩展 BPB (Extended BPB)**                    |
| 0x54           | 426              | 引导代码 (Bootstrap Code)                      |
| 0x1FE          | 2                | 结束标志，固定为 `55 AA`                       |

# BPB 详细字段分析 (偏移 0x0B - 0x23)

这部分字段在格式上与早期的 FAT 文件系统保持兼容，但某些字段在 NTFS 中被固定为 0。

| **偏移** | **长度** | **字段名称**      | **说明**                                   |
| -------- | -------- | ----------------- | ------------------------------------------ |
| 0x0B     | 2        | 每扇区字节数      | 常见值为 `512` (0x0200) 或 `4096` (0x1000) |
| 0x0D     | 1        | 每簇扇区数        | 簇的大小。通常为 8 (即每簇 4KB)            |
| 0x0E     | 2        | 保留扇区数        | **固定为 0** (NTFS 不使用此字段)           |
| 0x10     | 3        | FAT 数量 / 0      | **固定为 0**                               |
| 0x11     | 2        | 根目录条目数      | **固定为 0** (NTFS 根目录在 MFT 中)        |
| 0x13     | 2        | 总扇区数 (小)     | **固定为 0**                               |
| 0x15     | 1        | 介质描述符        | 硬盘通常为 `F8`                            |
| 0x16     | 2        | 每个 FAT 的扇区数 | **固定为 0**                               |
| 0x18     | 2        | 每道扇区数        | 用于旧的 CHS 寻址                          |
| 0x1A     | 2        | 磁头数            | 用于旧的 CHS 寻址                          |
| 0x1C     | 4        | 隐藏扇区数        | 分区起始位置之前的扇区数 (LBA 地址)        |
| 0x20     | 4        | 总扇区数 (大)     | **固定为 0** (NTFS 在扩展 BPB 中定义此值)  |



| **字段名称**                                         | **偏移量 (字节)** | **大小 (位)** | **典型值**                     | **用途**                                                     |
| ---------------------------------------------------- | ----------------- | ------------- | ------------------------------ | ------------------------------------------------------------ |
| **每扇区字节数** (Bytes per sector)                  | 11–12             | 16            | 512                            | 指定每个扇区的字节数，定义了读写操作的最小单位；有效值为 512, 1024, 2048 或 4096。 |
| **每簇扇区数** (Sectors per cluster)                 | 13                | 8             | 1, 2, 4, 8 (1到128之间的2的幂) | 表示组合成一个分配单元（簇）的扇区数量。通过减少小文件的 FAT 开销来优化存储效率。 |
| **保留扇区数** (Reserved sectors)                    | 14–15             | 16            | 1                              | 计算卷开头保留的扇区数（在 FAT 开始之前），通常仅包含引导扇区本身；该值必须至少为 1。 |
| **FAT 副本数** (Number of FATs)                      | 16                | 8             | 2                              | 定义卷上 FAT 副本的数量，用于冗余备份以及错误恢复期间的可靠性。 |
| **根目录条目数** (Root directory entries)            | 17–18             | 16            | 512 (适用于 FAT12/16)          | 指定根目录中 32 字节条目的最大数量。目录大小为该值乘以 32 字节。在某些扩展变体中该值为 0。 |
| **总扇区数 (短)** (Total sectors - small)            | 19–20             | 16            | 视情况而定 (32 MB 以下卷)      | 提供卷上总扇区的 16 位计数；如果使用 32 位版本，则此项设为 0。 |
| **介质描述符** (Media descriptor)                    | 21                | 8             | 0xF8 (硬盘), 0xF0 (软盘)       | 标识介质类型，影响 FAT 缓存和中断行为；常见值为 0xF0–0xFF，对应不同的存储介质。 |
| **每个 FAT 的扇区数 (短)** (Sectors per FAT - small) | 22–23             | 16            | 取决于卷大小                   | 给出每个 FAT 占用的 16 位扇区计数；决定了 FAT 的大小。使用 32 位扩展版本时设为 0。 |
| **每磁道扇区数** (Sectors per track)                 | 24–25             | 16            | 63 (硬盘常用值)                | 描述 BIOS 几何模型中每磁道的扇区数，用于传统的 BIOS 中断 13h 磁盘访问。 |
| **磁头数** (Number of heads)                         | 26–27             | 16            | 16 或 255 (硬盘常用值)         | 表示 BIOS CHS（柱面-磁头-扇区）寻址方案中的磁头数，用于兼容早期硬件。 |
| **隐藏扇区数** (Hidden sectors)                      | 28–31             | 32            | 视情况而定 (如分区偏移量)      | 计算卷开始之前的隐藏扇区数（例如前一个分区或 0 磁道的扇区）；对于非分区介质（如软盘）为 0。 |
| **总扇区数 (长)** (Total sectors - large)            | 32–35             | 32            | 视情况而定 (32 MB 以上卷)      | 为较大卷提供 32 位总扇区计数；当 16 位字段为 0 时使用，支持超过 32 MB 的容量。 |

# 扩展 BPB 详细字段分析 (偏移 0x24 - 0x53)

这是 NTFS 真正核心的部分，定义了 MFT（主文件表）的位置。

| **偏移** | **长度** | **字段名称**        | **说明**                                                  |
| -------- | -------- | ------------------- | --------------------------------------------------------- |
| 0x24     | 4        | 未使用              | 固定为 `80 00 80 00`                                      |
| 0x28     | 8        | **总扇区数**        | 分区的精确扇区总量 (64位整数)                             |
| 0x30     | 8        | **$MFT 的 LCN**     | 主文件表 MFT 的起始逻辑簇号                               |
| 0x38     | 8        | **$MFTMirr 的 LCN** | MFT 镜像（前4个记录）的起始逻辑簇号                       |
| 0x40     | 1        | 每个 MFT 记录的大小 | 若为正值，表示簇数；若为负值 (如 `F6`)，其绝对值表示 $2^{ |
| 0x41     | 3        | 未使用              | 填充                                                      |
| 0x44     | 1        | 每个索引缓冲的大小  | 用于 B+ 树索引记录的大小，计算方式同上                    |
| 0x45     | 3        | 未使用              | 填充                                                      |
| 0x48     | 8        | 分区序列号          | 格式化时生成的随机 ID (Volume Serial Number)              |
| 0x50     | 4        | 校验和              | 引导扇区的循环冗余检查值 (通常不强制检查)                 |



| 字段名称                     | 偏移量 | 大小（字节） | 描述                                                         |
| ---------------------------- | ------ | ------------ | ------------------------------------------------------------ |
| 物理驱动器号 (BS_DrvNum)     | 36     | 1            | 一个8位值，表示物理驱动器，例如0x80表示第一个硬盘，或0x00表示软盘；引导代码在调用中断13h时使用它来识别引导设备。[1][18] |
| 保留 (NT) (BS_Reserved1)     | 37     | 1            | 一个8位Windows特定字段，作为与可引导分区和卷完整性检查相关的标志；在卷创建期间通常设置为0x00。[18] |
| 签名 (BS_BootSig)            | 38     | 1            | 一个8位值，必须为0x29才能验证以下EBPB字段的存在。[1]         |
| 卷序列号 (BS_VolID)          | 39–42  | 4            | 一个32位唯一标识符，通过将当前时间戳（格式化时的日期和时间）与伪随机元素组合生成，以确保卷在系统间的唯一性。[1][18] |
| 卷标 (BS_VolLab)             | 43–53  | 11           | 表示卷名称的ASCII字符串，如果短于11个字符则用空格填充；如果未指定标签，则默认为"NO NAME    "，与根目录条目匹配。[1][18] |
| 文件系统类型 (BS_FilSysType) | 54–61  | 8            | 一个信息性ASCII字符串，用于标识文件系统，例如"FAT12   "或"FAT32   "；Microsoft驱动程序不使用它来确定实际FAT类型，但有助于兼容性。[1][18] |

# 关键点总结

- **MFT 是核心**：偏移 `0x30` 指向了 MFT 的位置。MFT 是整个 NTFS 的“大管家”，如果找不到它，整个分区就无法读取。
- **簇大小 (Cluster Size)**：由 `0x0B` (每扇区字节) 乘以 `0x0D` (每簇扇区数) 决定。绝大多数 Windows 默认格式化的结果是 **4 KB**。
- **备份机制**：NTFS 会在分区的 **最后一个扇区** 存放一份引导扇区的完整副本。如果分区的开头被破坏，可以通过末尾的备份进行恢复。

# Code

```c

#pragma pack(push, 1)  // Ensure that the structure is not populated with bytes

typedef struct _PACKED_NTFS_BIOS_PARAMETER_BLOCK  // NTFS BIOS Parameter Block Structure
{
    /** offset 0x00 */  UCHAR             bytes_per_sector[2];   // Bytes per sector
    /** offset 0x02 */  UCHAR          sectors_per_cluster[1];  // Sectors per cluster
    /** offset 0x03 */  UCHAR             reserved_sectors[2];  // Reserved sectors
    /** offset 0x05 */  UCHAR               number_of_fats[1];  // Number of FATs ,always 0 for NTFS
    /** offset 0x06 */  UCHAR                 root_entries[2];  // Root entries ,always 0 for NTFS
    /** offset 0x08 */  UCHAR          total_sectors_short[2];  // Total sectors ,if zero, use total_sectors_long
    /** offset 0x0A */  UCHAR             media_descriptor[1];  // Media descriptor
    /** offset 0x0B */  UCHAR        sectors_per_fat_short[2];  // Sectors per FAT ,always 0 for NTFS
    /** offset 0x0D */  UCHAR            sectors_per_track[2];  // Sectors per track
    /** offset 0x0F */  UCHAR              number_of_heads[2];  // Number of heads
    /** offset 0x11 */  UCHAR               hidden_sectors[4];  // Hidden sectors
    /** offset 0x15 */  UCHAR           total_sectors_long[4];  // Total sectors if total_sectors_short is zero
}NTFS_BIOS_PARAMETER_BLOCK, *PNTFS_BIOS_PARAMETER_BLOCK;        // sizeof = 0x019 25 bytes
#pragma pack(pop)  // Restore the default alignment

```

