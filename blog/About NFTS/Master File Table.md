# NTFS Master File Table
![](../picture/Pasted image 20260106184209.png

包含从 NTFS 分区检索文件所需的信息，例如文件的属性。

NTFS 卷上的每个文件都由一个名为主文件表 (MFT) 的特殊文件中的一条记录表示。**NTFS 保留该表的前 16 条记录用于存储特殊信息**。 例如:

- 0：MFT（$Mft）
- 5：根目录 （\）
- 6：卷集群分配文件（$Bitmap）
- 8：坏集群文件（$BadClus）



该表的第一条记录描述了主文件表本身，后面跟着一条 MFT 镜像记录。

如果第一个 MFT 记录损坏，NTFS 会读取第二个记录来查找 MFT 镜像文件，该文件的第一个记录与 MFT 的第一个记录相同。MFT 和 MFT 镜像文件的数据段位置都记录在引导扇区中。

![image-20251219173525901](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20251219173525901.png)

主文件表为每个文件记录分配一定空间。文件的属性会被写入主文件表分配的空间。小型文件和目录（通常为 512 字节或更小），例如下图所示的文件，可以完全包含在主文件表记录中。

 对于 NTFS 文件系统卷上的每个文件，MFT 中至少有一个条目，包括 MFT 本身。 有关文件的所有信息（包括其大小、时间和日期戳、权限和数据内容）都存储在 MFT 条目中，或存储在 MFT 条目描述的 MFT 之外的空间中。

  随着文件被添加到 NTFS 文件系统卷，更多的条目被添加到 MFT 中，MFT 的大小也会增加。 从 NTFS 文件系统卷中删除文件时，其 MFT 条目标记为空闲，可以重复使用。 但是，为这些条目分配的磁盘空间不会重新分配，MFT 的大小也不会减小。

NTFS 文件系统为 MFT 保留空间，以使 MFT 在增长时尽可能保持连续。 NTFS 文件系统在每个卷中为 MFT 保留的空间称为 MFT 区域。 文件和目录的空间也从该空间分配，但只有在 MFT 区域外的所有卷空间都已分配之后。



默认 MFT 区域由系统在装载卷时计算和保留，并基于卷大小。 可以通过 [NTFS 如何为其主文件表 (MFT) 预留空间](https://learn.microsoft.com/zh-cn/troubleshoot/windows-server/backup-and-storage/ntfs-reserves-space-for-mft)中详细介绍的注册表项，来增加 MFT 区域，但是不能使默认的 MFT 区域小于计算的空间。 增加 MFT 区域不会减少用户可用于数据文件的磁盘空间。



## MFT Record for a Small File or Directory

![image-20251219174146996](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20251219174146996.png)

这种设计使得文件访问速度非常快。例如，FAT 文件系统使用文件分配表(FAT)来列出每个文件的**名称和地址**。FAT 目录项包含指向文件分配表的索引。

当您想要查看某个文件时，FAT 文件系统首先会读取文件分配表 (FAT) 并确认该文件存在。然后，FAT 会通过搜索分配给该文件的分配单元链来检索该文件。而 NTFS 文件系统则不同，一旦您找到该文件，它就在那里供您使用。

目录记录与文件记录一样，都存储在主文件表中。不同之处在于，目录包含的是索引信息，而不是数据。

小型目录记录完全位于 MFT 结构内。大型目录则组织成 B 树，其记录包含指向外部集群的指针，这些外部集群包含 MFT 结构无法容纳的目录条目

# 文件记录段

每个文件记录段都以一个文件记录段头开始。`FILE_RECORD_SEGMENT_HEADER` 见 [FILE_RECORD_SEGMENT_HEADER](./FILE_RECORD_SEGMENT_HEADER.md)

每个文件记录段后面都有一个或多个属性。每个属性都以一个属性记录头开始

# Master File Table Copy 

包括用于在原始副本出现问题时恢复文件系统所必需的记录副本


# B* tree

![](../picture/Pasted%20image%2020260106190806.png)