# __builtin_prefetch 详解

## 语法

```c
void __builtin_prefetch(const void *addr, int rw = 0, int locality = 3);
```

### 参数

- **`addr`**
  - 需要预取的内存地址。
  - CPU 会尝试把该地址附近的数据提前加载到缓存。
- **`rw`** （可选，默认 `0`）
  - `0` = 预取 **读**（read）
  - `1` = 预取 **写**（write）
- **`locality`** （可选，默认 `3`）
  - 表示数据在缓存中的“停留倾向”：
    - `0` = 不需要缓存（一次性使用）
    - `1` = 低局部性（很快用完）
    - `2` = 中等局部性
    - `3` = 高局部性（数据可能会反复访问）

### 示例

```c
void sum_array(int *arr, size_t n) {
    long long sum = 0;
    for (size_t i = 0; i < n; i++) {
        // 提前预取下一个缓存行的数据
        if (i + 16 < n) {
            __builtin_prefetch(&arr[i + 16], 0, 3);
        }
        sum += arr[i];
    }
}
```

➡️ 这样 CPU 会在访问 `arr[i]` 时，同时加载未来要访问的数据，减少缓存未命中（cache miss）。

------

#  其他常用 `__builtin_*`

## 1. 分支预测优化

```c
int __builtin_expect(long exp, long c);
```

- 用于告诉编译器某个分支的执行概率高。

- 常见封装：

  ```c++
  #define LIKELY(x)   __builtin_expect(!!(x), 1)
  #define UNLIKELY(x) __builtin_expect(!!(x), 0)
  ```

------

## 2. 位操作内建函数

GCC/Clang 提供了很多位操作加速函数，通常映射到一条 CPU 指令。

- `__builtin_clz(x)`：返回前导零的个数（count leading zeros）
- `__builtin_ctz(x)`：返回末尾零的个数（count trailing zeros）
- `__builtin_popcount(x)`：返回二进制中 1 的个数（population count）
- `__builtin_parity(x)`：返回奇偶校验（1 的个数是奇数返回 1，否则 0）

示例：

```c
int bits = __builtin_popcount(0b101101); // 结果 = 4
```

------

## 3. 内存和对象操作

- `__builtin_memcpy`, `__builtin_memset`, `__builtin_memcmp`
   编译器可优化成 `rep movs`/`rep stos` 或者 SIMD。
- `__builtin_object_size(ptr, type)`
   返回已知对象的大小（编译期能推导时），常用于安全函数替换。

------

## 4. 栈与返回地址

- `__builtin_return_address(level)`
   获取调用栈上的返回地址（调试/异常处理用）。
- `__builtin_frame_address(level)`
   获取栈帧指针。

> ⚠️ 这些和 ABI/平台相关，移植性差。

------

## 5. 原子操作（较低层）

GCC 提供：

- `__sync_fetch_and_add`, `__sync_bool_compare_and_swap` （旧 API）
- `__atomic_load_n`, `__atomic_store_n`, `__atomic_compare_exchange_n` （新 API，C11 memory model 对齐）

------

# 🔹 总结

- **`__builtin_prefetch`**：缓存优化，降低 cache miss
- **`__builtin_expect`**：分支预测提示
- **位操作内建函数**：高效位运算，通常映射为单条 CPU 指令
- **内存操作**：编译器替换成最优的汇编实现
- **原子操作**：底层并发原语，映射到 `LOCK` 指令

这些函数都属于 **性能优化/低层次系统编程** 的工具箱，写性能敏感库时很常见。

------

