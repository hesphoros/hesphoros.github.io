# RecyclingAllocator 设计哲学

本文说明 `LUSPKERNELCORE::Memory` 中 **Recycler** 与 **RecyclingAllocator** 的分层意图、与 LLVM 的对应关系，以及在本项目分配器体系（`SystemAllocatorBacking`、`ArenaAllocatorImpl`、`IAllocator`）下的行为约定。

**实现入口**：仓库根目录下 `include/Memory/RecyclingAllocator.hpp`。

---

## 1. 要解决什么问题

- **通用分配器**（堆、mimalloc、或 bump arena）擅长「向系统要一块内存」，但**高频、同尺寸、同对齐**的小对象若每次都走全新分配与释放，会带来缓存与分配器内部元数据开销。
    
- **Recycler** 在「固定槽位 `(Size, Alignment)`」下维护一条**侵入式空闲链表**：释放时不立刻把块还给系统，而是挂回链表；下次分配优先从链表弹出，不足时再向底层要一块 `Allocate(Size, Alignment)`。
    
- **RecyclingAllocator** 把上述逻辑与**具体后端**（`BackingT`）绑成一个对象，生命周期结束时**统一把空闲链上的块交还给后端**（或在后端为 no-op 时仅丢弃引用），语义上对齐 LLVM 的 `llvm::RecyclingAllocator`。
    

设计哲学可以概括为：**在类型与槽位维度上「池化」；在存储生命周期上仍服从底层分配器的规则**。

---

## 2. 两层结构：职责分离（与 Arena 一致直接使用 BackingT）

|组件|职责|
|---|---|
|**Recycler<T, Size, Alignment>**|只关心空闲链与「向传入的后端引用」申请/归还；**不拥有**底层分配器。要求：`Allocate(size, alignment)`、`Deallocate(void*)`（`clear` 路径）。|
|**RecyclingAllocator<T, BackingT, Size, Alignment>**|拥有 `Recycler` + `BackingT`；直接把 `m_backing` 传给 `Recycler`（与 `ArenaAllocatorImpl` 直接使用 `BackingT` 同一哲学）。|

**与 LLVM 对齐方式**：LLVM 的 `Recycler::clear` 对 malloc 系后端调用 `Deallocate(t, Size, Align)`。本项目中 **Recycler 统一只调用单参 `Deallocate(void*)`**；`SystemAllocatorBacking` 因此提供 **`Deallocate(void*)` 重载**（内部即 `Free`），并保留 **`Deallocate(ptr, size, alignment)`** 供 `ArenaAllocatorImpl` 释放 slab / 大块时沿用。二者语义一致（mimalloc 侧不依赖 size/align），无需再引入 `RecyclerBackingBridge` 之类的适配类型。

---

## 3. 侵入式空闲链与默认 Size / Alignment

- 空闲时，槽位内存的开头存放 `FreeNode{ next* }`，因此 **槽位至少能容纳一个指针**：默认 `Size` 取 `max(sizeof(T), sizeof(void*))`，默认 `Alignment` 取 `max(alignof(T), alignof(void*))`，避免小类型在 64 位平台上 `sizeof(T) < sizeof(void*)` 时无法合法嵌入链表头。
    
- **Allocate<SubClass>** 在编译期约束 `sizeof(SubClass) ≤ Size` 且 `alignof(SubClass) ≤ Alignment`，保证多态层级中「子类塞进父类槽位」的安全边界由模板静态检查保证。
    

这是典型的 **空间换管理与可预测性**：槽位略大于最小 `T` 时，换来统一的回收路径与更少的外部元数据。

---

## 4. 生命周期与所有权：谁真正拥有内存

|操作|行为|
|---|---|
|**Allocate**|优先从空闲链取块；否则调用后端 `Allocate(Size, Alignment)`。返回的是**未构造**的存储（与 LLVM 一致）。|
|**Deallocate**|**不调用析构函数**；仅把指针挂回空闲链。对象状态的销毁由调用方负责（显式析构或 POD 语义）。|
|**clear(allocator)**（Recycler）|弹出空闲链上每一块，对适配器调用 `Deallocate(void*)`；链表清空。|
|**~RecyclingAllocator**|`m_recycler.clear(m_backing)`，保证析构时不在 Recycler 里残留空闲节点（与 LLVM 析构里 `Base.clear(Allocator)` 一致）。|

**与 Arena 组合时**：Arena 的 `Deallocate(void*)` 通常为 **no-op**。此时 `clear` 只是把空闲链上的指针「放手」——**实际 slab 仍由 Arena 持有**，直到 Arena 被 `Reset` / 析构。这与 LLVM 对 BumpPtrAllocator 分支「直接丢弃空闲链、不逐个 Deallocate」的意图一致，只是当前实现仍可能遍历链表调用 no-op（行为正确，极端大空闲链时仅有常数倍开销，可后续优化为 Bump 特判）。

**与 SystemAllocatorBacking 组合时**：`clear` 调用 `Deallocate(void*)`，空闲链上的块会真正归还 mimalloc/系统路径。

---

## 5. 与 LLVM 的对应关系

|LLVM|本项目|
|---|---|
|`llvm::Recycler<T, Size, Align>`|`Recycler<T, Size, Alignment>`|
|`llvm::RecyclingAllocator<T, AllocatorType, Size, Align>`|`RecyclingAllocator<T, BackingT, Size, Alignment>`|
|`Recycler::clear(Allocator)`，Bump 特化直接清空链表|通用 `clear`；Bump 可后续做 O(1) 优化|
|`PrintStats` / `PrintRecyclerStats`|`PrintStats()` 仅在 `BackingT` 提供 `PrintStats()` 时转发；Recycler 侧用 `recycledCount()`|
|全局 `operator new` / `delete` 绑定 RecyclingAllocator|在**全局命名空间**提供 placement 形式（Clang 不允许放在普通命名空间内）|

命名上保留 `PrintStatus()` 作为 `PrintStats()` 别名，便于从其它代码库迁移。

---

## 6. 使用约束（误用防范）

1. **Deallocate 不析构**：若类型非平凡析构，须在归还前在**用户代码**中显式调用析构，否则会双重构造或泄漏状态。
    
2. **槽位同质**：同一 `Recycler` 实例上回收的指针必须来自同一 `(Size, Alignment)` 语义；不要混入更大或更高对齐的类型。
    
3. **Recycler 单独使用时**：析构前必须 `clear(allocator)` 或保证空闲链为空，否则 assert（与 LLVM「非空 Recycler 被删」一致）。
    
4. **placement new**：`operator new(size_t, RecyclingAllocator&)` 带 `assert(n <= Size)`，超出槽位的 `new` 是逻辑错误。
    

---

## 7. 小结

- **Recycler**：与后端解耦的「固定槽位对象池」逻辑，通过 `Allocate(size, align)` / `Deallocate(void*)` 接入后端引用。
    
- **RecyclingAllocator**：「后端 + 回收」的一体化句柄，**直接持有并传递 `BackingT`**，析构时 `clear(m_backing)`。
    
- **SystemAllocatorBacking**：同时提供单参 `Deallocate(void*)`（Recycler / 按指针释放）与三参 `Deallocate(ptr,size,align)`（Arena slab 路径），避免额外桥接类型。
    

该设计在哲学上延续 LLVM：**回收是策略层，真正内存来自可替换的 Allocator；类型与槽位在编译期钉死，运行期只做链表与按需下探分配。**

---

## 8. 相关文档

- Bump 与 slab 语义：`docs/LLVM_BumpPtrAllocator_知识文档.md`
    
- Arena 模版与后端：`docs/Allocator_模版化_Plan.md`