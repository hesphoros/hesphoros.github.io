## Formatted disk Layout
![Ntfs Sturcture](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/Pasted%20image%2020260106184606.png)

![image-20251219170034549](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20251219170034549.png)

![image-20251225021434327](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20251225021434327.png)

# 1. Boot Sector / VBR(volume boot recorder)
![](../picture/Pasted%20image%2020260106185145.png)
Boot sector 可引导分区，用于存储有关卷布局和文件系统结构的信息，以及加载 Ntdlr 的引导代码。

格式化 NTFS 卷时，格式化程序会将前 16 个扇区分配给 $Boot 元数据文件。实际上，第一个扇区是包含“引导”代码的引导扇区，接下来的 15 个扇区是引导扇区的 IPL（初始程序加载器）。为了提高文件系统的可靠性，NTFS 分区的最后一个扇区包含引导扇区的备用副本。

## NTFS Boot Sector NTFS 引导扇区

引导扇区由以下元素组成：

- 基于 x86 的 CPU 跳转指令。
- BIOS 参数块 (BPB)，一种数据结构
- 扩展BPB
- 启动操作系统的可执行引导代码（或引导程序代码）

All Windows Server 2003 boot sectors contain the preceding elements regardless of the type of disk (basic disk or dynamic disk).

---



| Byte Offset | Field Length | Field Name               |
| :---------: | :----------: | ------------------------ |
|    0x00     |   3 bytes    | Jump Instruction         |
|    0x03     |   LONGLONG   | OEM ID                   |
|    0x0B     |   25 bytes   | BPB(BIOS PARAMTER BLOCK) |
|    0x24     |   48 bytes   | Extended BPB             |
|    0x54     |  426 bytes   | Bootstrap Code(引导代码) |
|   0x01FE    |     WORD     | End of Sector Marker     |

在 NTFS 卷上，BPB 之后的数据字段构成扩展 BPB。这些字段中的数据使 Ntldr（NT 加载程序）能够在启动期间找到主文件表 (MFT)。与 FAT16 和 FAT32 卷不同，NTFS 卷上的 MFT 并不位于预定义的扇区中。因此，如果 MFT 的正常位置出现坏扇区，则可以移动 MFT。但是，如果数据损坏，则无法找到 MFT，Windows NT/2000 会认为该卷尚未格式化

引导扇区的末尾是一个 2 字节的结构，称为签名字或扇区结束标记，其值始终为 0x55AA。

下表描述了 NTFS 卷上 BPB 和扩展 BPB 中的字段。以 0x0B、0x0D、0x15、0x18、0x1A 和 0x1C 开头的字段与 FAT16 和 FAT32 卷上的相应字段相对应。示例值对应于本示例中的数据。

| Byte Offset | Field Length | Sample Value       | Field Name                                   |
| :---------: | :----------: | ------------------ | -------------------------------------------- |
|    0x0B     |     WORD     | 0x0002             | Bytes Per Sector                             |
|    0x0D     |     BYTE     | 0x08               | Sectors Per Cluster                          |
|    0x0E     |     WORD     | 0x0000             | Reserved Sectors                             |
|    0x10     |   3 BYTES    | 0x000000           | always 0                                     |
|    0x13     |     WORD     | 0x0000             | not used by NTFS                             |
|    0x15     |     BYTE     | 0xF8               | Media Descriptor                             |
|    0x16     |     WORD     | 0x0000             | always 0                                     |
|    0x18     |     WORD     | 0x3F00             | Sectors Per Track                            |
|    0x1A     |     WORD     | 0xFF00             | Number Of Heads                              |
|    0x1C     |    DWORD     | 0x3F000000         | Hidden Sectors                               |
|    0x20     |    DWORD     | 0x00000000         | not used by NTFS                             |
|    0x24     |    DWORD     | 0x80008000         | not used by NTFS                             |
|    0x28     |   LONGLONG   | 0x4AF57F0000000000 | Total Sectors                                |
|    0x30     |   LONGLONG   | 0x0400000000000000 | Logical Cluster Number for the file $MFT     |
|    0x38     |   LONGLONG   | 0x54FF070000000000 | Logical Cluster Number for the file $MFTMirr |
|    0x40     |    DWORD     | 0xF6000000         | Clusters Per File Record Segment             |
|    0x44     |     BYTE     | 0x01               | Clusters Per Index Buffer                    |
|    0x45     |   3 BYTES    | 0x000000           | not used by NTFS                             |
|    0x48     |   LONGLONG   | 0x14A51B74C91B741C | Volume Serial Number                         |
|    0x50     |    DWORD     | 0x00000000         | Checksum                                     |
|   0x01FE    |   2 BYTES    | 0x55AA             | End of sector marker                         |

以下示例展示了使用 Windows Server 2003 格式化的 NTFS 卷的引导扇区。打印输出分为三个部分：

- 字节 0x00–0x0A 是跳转指令和 OEM ID
- 字节 0x0B–0x53 是 BPB 和扩展 BPB。
- 剩余的代码是引导代码和扇区结束标记

```
Physical Sector: Cyl 0, Side 1, Sector 1                                    
00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....
```



## BIOS PARAMTER BLOCK

[BPB](./BPB.MD)
![](../picture/Pasted%20image%2020260106185150.png)