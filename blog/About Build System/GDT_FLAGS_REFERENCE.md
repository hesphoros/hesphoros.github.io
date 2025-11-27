# GDT 标志位快速参考

## 📋 访问字节 (Access Byte) - 8 位

### 位结构
```
 7    6  5    4    3    2    1    0
┌───┬────────┬───┬───┬────┬────┬───┐
│ P │  DPL   │ S │ E │ DC │ RW │ A │
└───┴────────┴───┴───┴────┴────┴───┘
```

### 位说明

| 位 | 名称 | 值 | 说明 |
|----|------|-----|------|
| **7** | **P (Present)** | 0/1 | **段是否存在于内存中**<br>• 1 = 存在 (必须)<br>• 0 = 不存在 (触发异常) |
| **6-5** | **DPL (Privilege)** | 00-11 | **描述符特权级别**<br>• 00 = Ring 0 (内核)<br>• 01 = Ring 1<br>• 10 = Ring 2<br>• 11 = Ring 3 (用户) |
| **4** | **S (Type)** | 0/1 | **段类型**<br>• 1 = 代码/数据段<br>• 0 = 系统段 (TSS, LDT) |
| **3** | **E (Executable)** | 0/1 | **是否可执行**<br>• 1 = 代码段<br>• 0 = 数据段 |
| **2** | **DC (Direction/Conforming)** | 0/1 | **方向/一致性**<br>代码段: 0=非一致, 1=一致<br>数据段: 0=向上扩展, 1=向下扩展 |
| **1** | **RW (Read/Write)** | 0/1 | **读写权限**<br>代码段: 0=只执行, 1=可读<br>数据段: 0=只读, 1=可写 |
| **0** | **A (Accessed)** | 0/1 | **访问位** (CPU 自动设置)<br>• 0 = 未访问<br>• 1 = 已访问 |

---

## 🚩 粒度/标志字节 (Granularity Byte) - 8 位

### 位结构
```
 7    6    5    4       3   2   1   0
┌───┬────┬───┬─────┬──────────────────┐
│ G │ D/B│ L │ AVL │  Limit[19:16]    │
└───┴────┴───┴─────┴──────────────────┘
```

### 位说明

| 位 | 名称 | 值 | 说明 |
|----|------|-----|------|
| **7** | **G (Granularity)** | 0/1 | **段限制的单位**<br>• 0 = 字节 (最大 1MB)<br>• 1 = 4KB 页 (最大 4GB) |
| **6** | **D/B (Size)** | 0/1 | **操作数/地址大小**<br>• 0 = 16 位模式<br>• 1 = 32 位模式 |
| **5** | **L (Long mode)** | 0/1 | **64 位模式**<br>• 0 = 32/16 位<br>• 1 = 64 位 (x86-64) |
| **4** | **AVL (Available)** | 0/1 | **系统可用位**<br>• 由操作系统自由使用<br>• CPU 不解释此位 |
| **3-0** | **Limit[19:16]** | 0-F | **段限制的高 4 位** |

---

## 🎯 常用组合

### 标准 32 位保护模式段

#### 1️⃣ 内核代码段 (Ring 0)
```c
Access = 0x9A  // 1001 1010
= GDT_ACCESS_PRESENT      // 1... ....  段存在
| GDT_ACCESS_RING0        // .00. ....  Ring 0
| GDT_ACCESS_CODE_DATA    // ...1 ....  代码/数据段
| GDT_ACCESS_EXECUTABLE   // .... 1...  可执行
| GDT_ACCESS_RW           // .... ..1.  可读

Flags = 0xCF   // 1100 1111
= GDT_FLAG_GRANULARITY    // 1... ....  4KB 粒度
| GDT_FLAG_32BIT          // .1.. ....  32 位
| 0x0F                    // .... 1111  Limit 高 4 位

用途: 内核代码 (可执行, 可读, 不可写)
```

#### 2️⃣ 内核数据段 (Ring 0)
```c
Access = 0x92  // 1001 0010
= GDT_ACCESS_PRESENT      // 1... ....  段存在
| GDT_ACCESS_RING0        // .00. ....  Ring 0
| GDT_ACCESS_CODE_DATA    // ...1 ....  代码/数据段
| GDT_ACCESS_RW           // .... ..1.  可写

Flags = 0xCF   // 1100 1111
= GDT_FLAG_GRANULARITY    // 1... ....  4KB 粒度
| GDT_FLAG_32BIT          // .1.. ....  32 位
| 0x0F                    // .... 1111  Limit 高 4 位

用途: 内核数据/栈 (可读可写, 不可执行)
```

#### 3️⃣ 用户代码段 (Ring 3)
```c
Access = 0xFA  // 1111 1010
= GDT_ACCESS_PRESENT      // 1... ....  段存在
| GDT_ACCESS_RING3        // .11. ....  Ring 3
| GDT_ACCESS_CODE_DATA    // ...1 ....  代码/数据段
| GDT_ACCESS_EXECUTABLE   // .... 1...  可执行
| GDT_ACCESS_RW           // .... ..1.  可读

Flags = 0xCF   // 1100 1111

用途: 用户程序代码 (可执行, 可读, 不可写)
```

#### 4️⃣ 用户数据段 (Ring 3)
```c
Access = 0xF2  // 1111 0010
= GDT_ACCESS_PRESENT      // 1... ....  段存在
| GDT_ACCESS_RING3        // .11. ....  Ring 3
| GDT_ACCESS_CODE_DATA    // ...1 ....  代码/数据段
| GDT_ACCESS_RW           // .... ..1.  可写

Flags = 0xCF   // 1100 1111

用途: 用户程序数据/栈 (可读可写, 不可执行)
```

---

## 🔢 段限制计算

### 字节粒度 (G=0)
```
实际大小 = limit + 1 (字节)
最大限制 = 0xFFFFF = 1,048,575 字节 ≈ 1 MB
```

**示例:**
- `limit = 0x1000` → 大小 = 4,097 字节 (4KB + 1 字节)
- `limit = 0xFFFFF` → 大小 = 1,048,576 字节 (1 MB)

### 4KB 页粒度 (G=1)
```
实际大小 = (limit + 1) × 4096 (字节)
最大限制 = 0xFFFFF × 4096 = 4,294,967,296 字节 = 4 GB
```

**示例:**
- `limit = 0x1000` → 大小 = 4,097 × 4096 = 16,781,312 字节 ≈ 16 MB
- `limit = 0xFFFFF` → 大小 = 1,048,576 × 4096 = 4 GB

### 完整 4GB 段配置
```c
base  = 0x00000000  // 起始地址 0
limit = 0xFFFFF     // 最大限制值
flags = 0xCF        // G=1 (4KB 粒度), D/B=1 (32位)

实际范围: 0x00000000 - 0xFFFFFFFF (完整 4GB 地址空间)
```

---

## ⚠️ 重要注意事项

### 代码段 vs 数据段

| 特性 | 代码段 (E=1) | 数据段 (E=0) |
|------|-------------|-------------|
| **可执行** | ✅ 总是可执行 | ❌ 永不可执行 |
| **可读** | RW=1 时可读 | ✅ 总是可读 |
| **可写** | ❌ 永不可写 | RW=1 时可写 |
| **DC 位** | 一致性 (Conforming) | 方向 (Direction) |

### Ring 权限规则

```
Ring 0 (内核)   ──> 可访问所有资源
  ↓
Ring 1          ──> 受限访问
  ↓
Ring 2          ──> 更受限
  ↓
Ring 3 (用户)   ──> 最受限，需系统调用访问内核
```

**规则:**
- 低权限代码**不能**直接访问高权限段
- 高权限代码**可以**访问低权限段
- 系统调用通过调用门或中断从 Ring 3 进入 Ring 0

### 段访问检查

```
CPL (Current Privilege Level)   当前代码的权限级别
DPL (Descriptor Privilege Level) 目标段的权限级别
RPL (Requested Privilege Level)  请求的权限级别

访问规则:
  max(CPL, RPL) ≤ DPL  (数据段)
  CPL ≤ DPL            (非一致代码段)
  CPL ≥ DPL            (一致代码段)
```

---

## 📊 访问字节速查表

| 段类型 | Access | 二进制 | 说明 |
|--------|--------|--------|------|
| **空描述符** | 0x00 | 0000 0000 | 必须存在，不可使用 |
| **内核代码 R0** | 0x9A | 1001 1010 | Present, Ring0, Code, Exec, Read |
| **内核数据 R0** | 0x92 | 1001 0010 | Present, Ring0, Data, Write |
| **用户代码 R3** | 0xFA | 1111 1010 | Present, Ring3, Code, Exec, Read |
| **用户数据 R3** | 0xF2 | 1111 0010 | Present, Ring3, Data, Write |
| **TSS (32位)** | 0x89 | 1000 1001 | Present, Ring0, Available TSS |
| **LDT** | 0x82 | 1000 0010 | Present, Ring0, LDT |

---

## 🛠️ 使用示例

### 创建平坦内存模型 (Flat Memory Model)
```c
// 所有段覆盖 0-4GB，依赖分页进行保护

// Entry 0: Null descriptor
gdt_set_entry(0, 0, 0, 0, 0);

// Entry 1: Kernel code (0-4GB, Ring 0)
gdt_set_entry(1, 0x00000000, 0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | 
              GDT_ACCESS_CODE_DATA | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
              GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);

// Entry 2: Kernel data (0-4GB, Ring 0)
gdt_set_entry(2, 0x00000000, 0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | 
              GDT_ACCESS_CODE_DATA | GDT_ACCESS_RW,
              GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);

// Entry 3: User code (0-4GB, Ring 3)
gdt_set_entry(3, 0x00000000, 0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | 
              GDT_ACCESS_CODE_DATA | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
              GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);

// Entry 4: User data (0-4GB, Ring 3)
gdt_set_entry(4, 0x00000000, 0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | 
              GDT_ACCESS_CODE_DATA | GDT_ACCESS_RW,
              GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
```

### 段选择器格式
```
 15                 3  2  1  0
┌────────────────────┬──┬────┐
│   Index (13 bits) │TI│ RPL│
└────────────────────┴──┴────┘

Index: GDT/LDT 中的索引 (0-8191)
TI:    表指示器 (0=GDT, 1=LDT)
RPL:   请求的特权级别 (0-3)
```

**示例:**
- `0x08 = 0000 1000` → Index=1, TI=0 (GDT), RPL=0 → 内核代码段
- `0x10 = 0001 0000` → Index=2, TI=0 (GDT), RPL=0 → 内核数据段
- `0x1B = 0001 1011` → Index=3, TI=0 (GDT), RPL=3 → 用户代码段 (Ring 3)

---

## 📚 参考资料

- **Intel 64 and IA-32 Architectures Software Developer's Manual, Volume 3A**
  - Chapter 3: Protected-Mode Memory Management
  - Section 3.4: Segment Descriptors
  
- **OSDev Wiki**
  - [GDT Tutorial](https://wiki.osdev.org/GDT_Tutorial)
  - [Segmentation](https://wiki.osdev.org/Segmentation)

---

**最后更新:** 2025-11-01  
**维护者:** HesphorosOS Team
