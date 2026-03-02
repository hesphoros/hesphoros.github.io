# FILE_RECORD_SEGMENT_HEADER

![](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/Pasted%20image%2020260106191441.png)

表示文件记录段。这是主文件表（MFT）中每个文件记录段的头部。

```c++
typedef struct _FILE_RECORD_SEGMENT_HEADER {
  MULTI_SECTOR_HEADER   MultiSectorHeader;
  ULONGLONG             Reserved1;
  USHORT                SequenceNumber;
  USHORT                Reserved2;
  USHORT                FirstAttributeOffset;
  USHORT                Flags;
  ULONG                 Reserved3[2];
  FILE_REFERENCE        BaseFileRecordSegment;
  USHORT                Reserved4;
  UPDATE_SEQUENCE_ARRAY UpdateSequenceArray;
} FILE_RECORD_SEGMENT_HEADER, *PFILE_RECORD_SEGMENT_HEADER;
```

![image-20251220184643080](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20251220184643080.png)

# code

```c
/**
 * @brief NTFS 文件系统中主文件表（MFT）中单个文件记录段头部
 * @details 
 * * 每个 FRS 的基本元数据
 * * 多扇区保护
 * * 日志序列号
 * * 属性偏移
 * * 引用计数
 * @param MULTI_SECTOR_HEADER 
 * * 多扇区保护机制头部，保证 FRS 在跨多个扇区时的数据完整性
 * @param log_file_sequence_number 
 * * 记录上一次修改该 FRS 的日志序列号
 * @param sequence_number 
 * * FRS 的序列号 每当一个文件记录段被释放时，该字段就会递增，且不使用 0
 * * 文件引用（File Reference）中的 SequenceNumber 字段必须与该字段的内容相匹配，
 * * 否则该文件引用就是错误的（通常认为已过期/失效）。
 * @param reference_count 
 * * 从 INDEX_xxx 属性中指向该记录段的引用计数在除主文件记录段
 * * （Base File Record Segment）之外的其他文件记录段中，此字段的值为 0。
 * @param first_attribute_offset
 * * FRS 中第一个属性（Attribute）记录的偏移量
 * @param fps_flags
 * * 文件记录段标志，指示该 FRS 的状态（如是否正在使用中，是否为目录等）
 * * `FILE_RECORD_SEGMENT_IN_USE` : 该记录段当前正在使用中
 * * `FILE_FILE_NAME_INDEX_PRESENT` : 该文件具有文件名索引(即它是一个目录)
 * @param first_free_byte_offset
 * * FRS 中第一个可用字节的偏移量,从 FRS 头部开始计算，通常四字节对齐
 * @param bytes_available
 * * FRS 中可用的总字节数 即整个 FRS 大小
 * @param base_file_record_segment
 * * 如果该 FRS 是一个扩展记录段（Extension Record Segment）
 * * 则该字段指向其基文件记录段（Base File Record Segment）
 * @param next_attribute_instance
 * * 创建新属性时的实例编号。
 * * 对初始属性（如 Standard Information）也需要实例编号
 * @param segment_number_high_part
 * @param segment_number_low_part
 * * 当前FRS在MFT中的段号（Segment Number） 
 * * 由高16位和低32位组成，用于唯一标识FRS在MFT中的位置
 * * 主要用于恢复(recovery)和一致性检查(consistency checking)
 * @param update_sequence_array
 * * 多扇区保护机制的更新序列数组(USA)
 * * 用于保护 FRS 在多扇区写入时的完整性，防止扇区损坏
 */
typedef struct _FILE_RECORD_SEGMENT_HEADER {
    /** offset 0x000 */  MULTI_SECTOR_HEADER        multi_sector_header;
    /** offset 0x008 */  LCN                   log_file_sequence_number; 
    /** offset 0x010 */  USHORT                         sequence_number; 
    /** offset 0x012 */  USHORT                         reference_count;
    /** offset 0x014 */  USHORT                  first_attribute_offset;
    /** offset 0x016 */  USHORT                               fps_flags;
    /** offset 0x018 */  ULONG                   first_free_byte_offset;
    /** offset 0x01C */  ULONG                          bytes_available;
    /** offset 0x020 */  MFT_SEGMENT_REFERENCE base_file_record_segment;
    /** offset 0x028 */  USHORT                 next_attribute_instance;
    /** offset 0x02A */  USHORT                segment_number_high_part;
    /** offset 0x02C */  ULONG                  segment_number_low_part;
    /** offset 0x030 */  UPDATE_SEQUENCE_ARRAY    update_sequence_array;
}FILE_RECORD_SEGMENT_HEADER, *PFILE_RECORD_SEGMENT_HEADER;
```

