# atomic
##   `std::atomic`

- `std::atomic` 是 C++11 引入的模板类，提供了**原子操作**，即在多线程环境下对变量的操作不会被线程切换中断，保证数据一致性和线程安全。
- 原子操作不需要显式加锁（比如 `std::mutex`），性能更高，适合一些简单的计数器、状态位等场景。
### 原子操作特性
- **不可分割性**: 操作要么完全执行，要么完全不执行，不存在部分执行的状态
- **一致性**: 保证数据的一致性状态
- **隔离性**: 一个原子操作执行时不会被其他操作中断
- **并发安全**: 多线程环境下可以安全使用，避免竞态条件

##  常见声明方式

```c++
#include <atomic>
std::atomic<T>              // 通用模板
std::atomic_bool            // bool的原子版本
std::atomic_char            // char的原子版本
std::atomic_int             // int的原子版本
std::atomic_uint            // unsigned int的原子版本
std::atomic_long            // long的原子版本
std::atomic_ulong           // unsigned long的原子版本
std::atomic_llong           // long long的原子版本
std::atomic_ullong          // unsigned long long的原子版本
std::atomic_size_t          // size_t的原子版本

// ptrdiff_t 是两个指针相减时的结果类型 通常是带符号long long
// 使用它可以安全在多线程下更新 指针差值 和 数组索引
std::atomic_ptrdiff_t       // ptrdiff_t的原子版本

// 常用于实现 无锁数据结构 需要把指针转成整数来做 CAS（compare-and-swap）操作
std::atomic_intptr_t        // intptr_t的原子版本

// 无符号整数版的指针大小类型
// 常见用途同上，比如存放指针地址或做指针位运算（如标记位
// 在lock-free 算法中经常会把指针 + 一些标志位一起打包到 `uintptr_t` 里
std::atomic_uintptr_t       // uintptr_t的原子版本
```

# 基本操作

```c++
// 构造函数
std::atomic<int> a;         // 默认初始化为0
std::atomic<int> a(5);      // 初始化为5

// T load(std::memory_order order = std::memory_order_seq_cst) const noexpect
// T load(std::memory_order order 
//           = std::memory_order_seq_cst ) const volatile noexcept
// 加载和存储
int x = a.load();           // 原子读取

// void store(T desired, std::memory_order order =
//              std::memory_order_seq_cst ) noexpect
// vodd store(T desired, std::memory_order order =
//              std::memory_order_seq_cst ) volatile noexcept
a.store(10);                // 原子写入

// 读-修改-写操作
// T exchange( T desired, std::memory_order order = 
//               std::memory_order_seq_cst ) noexcept
// T exchange( T desired, std::memory_order order= 
//               std::memory_order_seq_cst ) volatile noexcept
int old_value = a.exchange(20);  // 替换值并返回旧值
bool success = a.compare_exchange_weak(expected, desired);  // 比较并交换
bool success = a.compare_exchange_strong(expected, desired); // 比较并交换(强保证版)

// 算术和位操作
a.fetch_add(5);             // 原子加法，返回旧值
a.fetch_sub(3);             // 原子减法，返回旧值
a.fetch_and(0xF);           // 原子按位与
a.fetch_or(0x10);           // 原子按位或
a.fetch_xor(0xFF);          // 原子按位异或

// 也支持运算符重载
a += 5;                     // 相当于a.fetch_add(5) + 5
a -= 3;                     // 相当于a.fetch_sub(3) - 3
a &= 0xF;                   // 相当于a.fetch_and(0xF) & 0xF
a |= 0x10;                  // 相当于a.fetch_or(0x10) | 0x10
a ^= 0xFF;                  // 相当于a.fetch_xor(0xFF) ^ 0xFF
```

# compare_exchange_xxx
`compare_exchange`（CAS，Compare-And-Swap）是原子地做「**如果当前值等于期望值就替换为新值**」的操作。返回值表示是否替换成功；失败时会把**当前实际值写回到 `expected`**。

## 区别
- `compare_exchange_strong(expected, desired, ...)`
  
    - **强版**，只有在值确实不相等时才返回失败（不会（或极少）发生伪失败）。适合**单次尝试**或需要强保证的场景。
    
- `compare_exchange_weak(expected, desired, ...)`
  
    - **弱版**，可能发生**伪失败（spurious failure）** — 即使值相等也可能返回 `false`。但是在很多 CPU（使用 LL/SC 实现）上弱版更快，**适合放在循环中重试**（常用于 CAS loop）。

```c++
// 默认内存顺序(seq_cst)
bool compare_exchange_strong ( T& expected, T desired ) noexcept;
bool compare_exchange_strong ( T& expected, T desired ,
		std::memory_order success,
		std::memory_order failure) noexcept;
bool compare_exchange_strong ( T& expected, T desired ,
		std::memory_order success,
		std::memory_order failure) volatile noexcept;

bool compare_exchange_weak   ( T& expected, T desired ) noexcept;
bool compare_exchange_weak   ( T& expected, T desired ,
		std::memory_order success.
		std::memory_order failure) volatile noexcept;
bool compare_exchange_weak   ( T& expected, T desired ,
		std::memory_order order = std::memory_order_seq_cst ) noexcept;
```
- `expected` 是引用（调用者传入），失败时被赋为 atomic 的当前值；成功时 `expected` 保持原来的期望值。
  
- 返回 `true` 表示替换成功；`false` 表示失败（并把实际值写回 `expected`）。

## 成功 return true
- 条件：原子变量当前值 `V` 与 `expected` (预期) 中保存的值相等（即 `V == expected`）。
  
- 动作：原子地把原子变量的值更新为 `desired`（`V -> desired`）。 (期望)
  
- 结果：
  
    - 函数返回 `true`。
      
    - `expected` 保持原先传入的值（不会被覆盖为 `V`，因为 `V` 就是 `expected`）。
    
- 举例（数值）：
  
    - 原子变量 `a` 当前为 `5`，`expected = 5`，`desired = 6`：
      
        - 调用后：`a == 6`，返回 `true`，`expected == 5`（仍旧是 5）。
## 失败
- 条件：原子变量当前值 `V` 与 `expected` 不相等（`V != expected`），或者（对于 `weak`）发生**伪失败**。
  
- 动作（正常失败，即值不等的情况）：
  
    - 不做写入（原子值不变）。
      
    - 把当前实际值 `V` 写回到 `expected`（**替换 expected 的内容**），以便调用者知道失败时实际的原子值是什么。
    
- 结果（正常失败）：
  
    - 函数返回 `false`。
      
    - `expected` 被更新为当前原子值 `V`（便于调用者基于新值重试或计算新的 `desired`）。
    
- 举例（数值）：
  
    - 原子变量 `a` 当前为 `6`，`expected = 5`，`desired = 7`：
      
        - 调用后：`a` 仍然为 `6`，返回 `false`，并且 `expected` 被改为 `6`

## weak的伪失败
- `compare_exchange_weak` 可能 **伪失败**：即便 `V == expected`，实现也可能返回 `false`（这是允许的，为了在某些硬件上提高性能）。
  
- **关于 `expected` 是否被修改**：
  
    - 不同实现行为可能不同：在伪失败的情况下，某些实现可能不修改 `expected`，也有可能修改（标准并不强制完全统一行为）。
      
    - 因此常见且安全的做法：把 `weak` 放到循环里重试（`while (!weak(...)) { /* 继续 */ }`），不要依赖伪失败时 `expected` 的具体改变规则。
    
- 实践建议：在循环里用 `compare_exchange_weak`；若只做一次尝试（不想重试），用 `compare_exchange_strong`

- 单次尝试 使用`compare_exchange_strong`
```c++
std::atomic<int> a{5};
int  expected = 5;
int  desired  = 6;

if ( a.compare_exchange_stong( expected, desired ) ) {
	// success a 从 5 -> 6 , expected 仍为5
}  else {
	
	// failure : a 未变, expected 被更新为a 当前值
}

```
- 循环重试 
```c++
std::atomic<int> a{0};
int old = a.load();
do {
    // 如果失败，old 会被更新为 a 的当前值（或在伪失败时可能未更新——循环仍能正确处理）
} while (!a.compare_exchange_weak(old, old + 1));

```

## **执行轨迹举例（两线程竞争）**：
- 初始：`a = 5`
  
- 线程 A: `old = 5` → 成功将 `a` 设为 `6`，返回 `true`。
  
- 线程 B: `old = 5` → CAS 失败（此时 `a == 6`），返回 `false`，`old` 被更新为 `6`（正常失败情形）。线程 B 重试，用新 `old` 继续尝试。

## 使用陷阱
- **误把 `expected` 当作只读参数**：失败时 `expected` 会被写回新的值，调用者应当意识到这是设计使然。
  
- **把字面量传给 `expected`**：必须传变量引用，不要写 `compare_exchange_strong(5, 6)`——这是错误。
  
- **用 `weak` 做单次尝试**：`weak` 可能伪失败，单次尝试时应使用 `strong`。
  
- **忽视 ABA 问题**：在无锁链表/栈场景中，单纯 CAS 可能受 ABA 问题影响（需要 tag/counter 或 hazard pointers 等手段解决

## ABA 问题

 [[ABA问题]] 

# 高级特性
## std::atomic_flag
最简单的原子类型, 保证无锁实现,可用于实现**自旋锁**
```c++
std::atomic_flag lock = ATOMIC_FLAG_INIT; // 必须初始化为清除状态

// 尝试获取锁
while(lock.test_and_set(std::memory_order_acquire))
    ; // 自旋等待

// 临界区...

// 释放锁
lock.clear(std::memory_order_release);
```
## is_lock_free
检查原子类型是否真正无锁实现
```c++
std::atomic<int> a;
if (a.is_lock_free()) {
    std::cout << "使用硬件原子指令实现" << std::endl;
} else {
    std::cout << "内部使用锁实现" << std::endl;
}
```

## 原子智能指针操作(c++20)
```c
std::atomic<std::shared_ptr<T>> ptr;
auto p = std::make_shared<T>();
ptr.store(p);
auto current = ptr.load();
```



## 常见使用场景

### 计数器
```c++
std::atomic<int> counter{0};

void increment() {
    counter.fetch_add(1, std::memory_order_relaxed);
}

int get_count() {
    return counter.load(std::memory_order_relaxed);
}
```

### 双重检查锁定模式

```c++
std::atomic<Singleton*> instance;
std::mutex init_mutex;

Singleton* get_instance() {
    Singleton* p = instance.load(std::memory_order_acquire);
    if (p == nullptr) {
        std::lock_guard<std::mutex> lock(init_mutex);
        p = instance.load(std::memory_order_relaxed);
        if (p == nullptr) {
            p = new Singleton();
            instance.store(p, std::memory_order_release);
        }
    }
    return p;
}
```

### 无锁数据结构

```c++
template <typename T>
class lock_free_stack {
    struct node {
        T data;
        node* next;
        node(const T& data) : data(data), next(nullptr) {}
    };
    
    std::atomic<node*> head;
    
public:
    lock_free_stack() : head(nullptr) {}
    
    void push(const T& data) {
        node* new_node = new node(data);
        new_node->next = head.load(std::memory_order_relaxed);
        while (!head.compare_exchange_weak(new_node->next, new_node,
                                          std::memory_order_release,
                                          std::memory_order_relaxed));
    }
    
    bool pop(T& result) {
        node* old_head = head.load(std::memory_order_acquire);
        while (old_head && 
              !head.compare_exchange_weak(old_head, old_head->next,
                                         std::memory_order_release,
                                         std::memory_order_relaxed));
        if (old_head) {
            result = old_head->data;
            delete old_head;
            return true;
        }
        return false;
    }
};
```

#### 性能考量

- 原子操作通常比互斥锁快，但仍比非原子操作慢
- `memory_order_relaxed`性能最好，但提供的保证最少
- `memory_order_seq_cst`(默认)提供最强的保证，但可能有性能损失
- 不同CPU架构对原子操作的硬件支持不同，影响性能表现

#### 注意事项

- 原子操作不等于线程安全，复合操作仍需正确同步
- 原子操作主要适用于简单的共享状态管理
- 过度依赖特定内存序可能导致微妙的并发bug
- `compare_exchange_weak`在某些平台可能出现"伪失败"
- 避免在热点路径上使用原子操作，除非必要