# CodeLayer 

## 常见宏定义（给编译器分支预测提示）

在很多高性能项目（Linux 内核、glibc、Boost 等）都会看到类似的封装

![image-20250920204647839](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250920204647839.png)

### __builtin_expect

#### 语法定义

```c
long __builtin_expect(long exp, long c);
```

**exp**：一个整型表达式（通常是条件判断的结果）。

**c**：你预测 exp 最可能等于的值（通常是 `0` 或 `1`）。

返回值：
 `__builtin_expect(exp, c)` **本质上就等于 exp 本身**，只是额外带了“预测信息”给编译器。

#### 基本用法

在 C/C++ 里常见的写法：

```c++
if (__builtin_expect(x, 0)) {
    // 编译器认为 x 大多数情况下 == 0
}

if (__builtin_expect(x, 1)) {
    // 编译器认为 x 大多数情况下 != 0
}

```

也就是说：

- `__builtin_expect((expr), 1)` → **expr 很可能为真**
- `__builtin_expect((expr), 0)` → **expr 很可能为假**

#### example

```c#
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
```

这里 `!!(x)` 是把表达式强制转换为 `0` 或 `1`，避免歧义。

#### 效果

`__builtin_expect` **不会改变代码逻辑**，只是编译器在生成汇编时：

- 把“预测为真”的分支放在主路径（fallthrough path），减少跳转。
- 把“不常见”的分支放远一点，甚至丢到冷区（`.cold` 段）。

示例（GCC -O2）：

```c#
if (__builtin_expect(x == 0, 0)) {
    foo();
} else {
    bar();
}
```

可能生成的汇编（简化）：

```c#
cmp eax, 0
jne .Lbar      ; 假设 x==0 不太可能，所以直接跳到 bar()
call foo
```

#### 注意事项

- 只是 **hint**，编译器/CPU 不一定采纳。
- 滥用可能适得其反（特别是预测错误时，分支代价更高）。
- 适合用于 **错误处理、极少发生的异常情况**。

## 去分支化（if-conversion / predication）

### (1) 三元运算符 + cmov

编译器有时会把三元运算符优化为 `cmov`（条件移动指令），这样就没有真正的分支：

```c
// 普通写法
int max1(int a, int b) {
    return (a > b) ? a : b;
}

// 编译器可能优化成（x86 汇编）：
mov eax, a
cmp a, b
cmovl eax, b
```

这种 **避免了条件跳转**，在分支不可预测时更快。

### (2) 查表优化（table lookup）

当条件有限时，可以用数组代替 if-else：

```c
// if-else 版本
int f(int v) {
    if (v == 0) return 10;
    if (v == 1) return 20;
    if (v == 2) return 30;
    return -1;
}

// 查表版本
int table[] = {10, 20, 30};
int f2(int v) {
    return (v >= 0 && v < 3) ? table[v] : -1;
}
```

### (3) 位运算消分支

布尔条件在 C/C++ 里就是 0/1，可以转化为算术：

```c
// 有分支
int abs1(int x) {
    return (x >= 0) ? x : -x;
}

// 去分支（位技巧）
int abs2(int x) {
    int mask = x >> 31;      // 符号位 -> 全 1 或 全 0
    return (x ^ mask) - mask;
}
```

在某些场景下，CPU 的流水线性能更好（但可读性差）。

##  冷热代码分离

C/C++17 之后，GCC/Clang 支持 `[[likely]]` / `[[unlikely]]` 属性（语法级替代宏）：

```c
if (x > 0) [[likely]] {
    // 常见分支
} else [[unlikely]] {
    // 少见分支
}
```

这比 `__builtin_expect` 更现代，可读性更好。

还可以把冷路径（异常、错误处理）单独抽到函数里：

```c
if (UNLIKELY(err)) {
    return handle_error();
}
```

这样编译器会把 `handle_error` 放到 `.cold` 段，减少 I-cache 压力。

## 实际使用技巧

**错误处理 → UNLIKELY**
 错误、断言失败、极少数 case，一般加 `UNLIKELY`。

```c++
if (UNLIKELY(ptr == nullptr)) return -1;
```

**频繁判断但高度随机 → 去分支化**
 比如 hash 表冲突时选择桶，分支难预测 → 用查表或 `cmov`。

**循环中的分支 → 考虑展开/向量化**

```c++
for (int i=0; i<n; ++i) {
    if (arr[i] > 0) sum += arr[i];
}
```

可以用 SIMD 指令处理一批，避免逐次分支。

**使用 PGO（Profile-Guided Optimization）**
 现代编译器支持基于运行时 profile 的优化，分支预测更精准

##  注意事项

- `LIKELY/UNLIKELY` **只是 hint**，不保证性能提升，过度使用可能适得其反。
- 如果条件非常容易预测（如循环计数器 `< n`），**不用写 hint**。
- 如果分支几乎均匀随机，hint 没用 → 尝试去分支化。
- 要用 **perf/VTune** 等工具验证（看 `branch-misses` 指标）