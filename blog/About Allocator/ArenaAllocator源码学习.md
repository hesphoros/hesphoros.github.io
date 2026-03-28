此文档的ArenaAllocator实现来自LLVM。

# 设计目标和语义
- 池式单调增长：内存像「游标」一样在 slab 里向后分配；**不按指针单独回收**整块语义上的「堆」，对象生命周期通常绑定到 allocator 或一次 `Reset()`
- **并非无限连续堆**：底层是多段 **slab**（块），每段有 `CurPtr（当前偏移指针）`/`End`（end pointer），语义上仍是 bump。
- 超过阈值的进行单次的分配：不塞进当前小 slab，而是 **独立的一块内存**（LLVM 里叫 custom-sized slab / 大块路径），避免浪费当前 slab 尾部空间。
- 可插拔的底层分配器：模版参数`AllocatorT`默认 `MallocAllocator`，可换成自定义实现

# 模版参数
```cpp
BumpPtrAllocatorImpl<
    AllocatorT    = MallocAllocator,   // 底层向谁要内存
    SlabSize      = 4096,              // 初始 slab 基准大小（字节）
    SizeThreshold = SlabSize,          // 超过此大小的单次分配走「独立 slab」
    GrowthDelay   = 128                // 每分配这么多块 slab 后，扩大下一块 slab
>
```
**静态约束**（编译期）：

- `SizeThreshold <= SlabSize`：保证「比 slab 还大的对象」必然走独立分配，而不会错误地期望塞进标准 slab。
    
- `GrowthDelay > 0`：避免除零或永不增长。**摊销** `malloc` 频率：slab 越开越大

# 核心数据（心智模型）

| 成员                 | 含义                                        |
| ------------------ | ----------------------------------------- |
| `CurPtr`           | 当前 slab 中**下一个可写字节**（bump 游标） current ptr |
| `End`              | 当前 slab 末尾（不可写区域上界）                       |
| `Slabs`            | 标准 slab 指针序列（`SmallVector<void*, 4>`）     |
| `CustomSizedSlabs` | `(指针, 实际分配大小)`，给超大单次分配用                   |
| `BytesAllocated`   | 用户请求的累计字节（用于统计/浪费分析）                      |
|                    |                                           |
|                    |                                           |
## Slab 实际大小：`computeSlabSize`
```c
// 逻辑：第 SlabIdx 块 slab 的字节数
SlabSize * (1 << min(30, SlabIdx / GrowthDelay))
```
- 每 `GrowthDelay` 块 slab，**逻辑尺寸翻倍**，上限 `2^30` 倍（防止指数失控）。
    
- **第 0 块**就是 `SlabSize`；块数多了以后单块变大，减少向系统分配器的调用次数。
    

**改进 Arena 时的要点**：若长期高负载，固定 4MiB slab 可能分配次数偏多；可引入「延迟增长」或阶梯增长，在**内存峰值**与 **syscall/分配器调用次数**之间折中。

#  `Allocate`（热路径）
逻辑概要：

1. `BytesAllocated += Size`（统计优先于失败与否，LLVM 如此实现）。
    
2. `AlignedPtr = alignAddr(CurPtr, Alignment)`（对齐在**当前 bump 位置**上算）。
    
3. 在 ASan 下可能增加 `RedZoneSize` 得到 `SizeToAllocate`。
    
4. 若 `AlignedPtr + SizeToAllocate <= End` 且 `CurPtr != nullptr`：**热路径** —— 更新 `CurPtr`，MSan/ASan 标记后返回。
    
5. 否则调用 **`AllocateSlow`**（冷路径，标 `NOINLINE` 减轻主路径体积）
**与对齐相关的细节**：

- 重载 `Allocate(Size, size_t Alignment)` 要求 `Alignment > 0`，`0` 非法；可用 `1`。
    
- 另有 `Allocate(Size, Align)` 使用强类型对齐。
    

**零大小分配**：注释写明 `Allocate(0, N)` **合法**，返回**非空指针但不可解引用**（满足「永不返回 nullptr」的约定的一部分场景）。我们当前实现是 `size==0` 返回 `nullptr`，若要对齐 LLVM 行为需单独决策（API 与调用方假设）。

# `AllocateSlow`
两种分支：
### 1 超大对象（`PaddedSize > SizeThreshold`）

- `PaddedSize = SizeToAllocate + Alignment - 1`（为对齐预留）。
    
- 向底层 `Allocate(PaddedSize, max_align_t)` 要一块**独占**内存。
    
- 记入 `CustomSizedSlabs`，在对齐后的地址上 `unpoison`/`msan` 后返回。
### 2 否则：换标准 slab

- `StartNewSlab()`：`computeSlabSize(Slabs.size())` 决定**本块实际大小**，分配后更新 `CurPtr`、`End`。
    
- 再在新 slab 上对齐、bump 一次（断言空间足够）。
# `Reset()` 行为
- 空 **custom-sized** 分配并释放。
    
- 若存在标准 slab：**保留第一块**，`CurPtr`/`End` 重置到第一块首址与 `SlabSize`（首块逻辑大小仍用 `computeSlabSize(0)` 参与 ASan poison 等）。
    
- **删除**其余标准 slab 并释放底层内存。
    
- `BytesAllocated = 0`。
    
## 移动构造 / 移动赋值

- 必须**搬空**源对象的 `CurPtr`/`End`/slab 列表，避免双释放。
    
- 析构时遍历 slab 按 **索引对应的 `computeSlabSize`** 调用 `Deallocate`。
    

**启示**：若我们增加移动语义，必须与「按块大小释放」规则一致，不能只 `free` 指针而不知大小（除非底层是 `free` 且与大小无关 —— mimalloc 的 `mi_free` 属于此类，但统计与调试仍建议记录大小）。

#  `Deallocate` 在 LLVM bump 里的含义
```c
void Deallocate(const void *Ptr, size_t Size, size_t /*Alignment*/) {
  __asan_poison_memory_region(Ptr, Size);
}
```

**不归还内存给系统**：在 ASan 下只是把区域标为不可访问；语义上「逻辑释放」，指针可能仍用于非解引用场景。与我们 `ArenaAllocator::Deallocate` 空操作在「不真正 free」上一致，LLVM 在工具链下多了一步 poison。

# 辅助API
- **`GetNumSlabs`**：标准 slab 数 + custom 块数。
    
- **`getTotalMemory` / `getBytesAllocated` / `PrintStats`**：常驻内存 vs 用户请求字节，用于观察碎片与浪费。
    
- **`identifyObject`**：给定指针反查落在哪块 slab 及偏移（调试、序列化标识等）。

# `SpecificBumpPtrAllocator`
- 持有 `BumpPtrAllocator`，关闭 red zone（因为要按步长遍历析构）。
    
- **`DestroyAll()`**：按 `T` 对齐遍历每块 slab（含当前 bump 边界 `CurPtr`），调用析构后 `Allocator.Reset()`。
    
- 提供类型化 `Allocate(num)` 与 `identifyObject` 转发。
    

**启示**：若我们要支持「arena 上批量析构」，需要**可遍历布局**（固定对齐、步长或额外元数据），与纯 bump 指针不完全相同。

#  `operator new` / `operator delete` 重载
```c
void *operator new(size_t Size, BumpPtrAllocatorImpl &Allocator);
void operator delete(void *, BumpPtrAllocatorImpl &);
```
placement new 形式：`new (allocator) T(...)` 常见用法；`delete` 第二参数为空操作，与 bump 语义一致。