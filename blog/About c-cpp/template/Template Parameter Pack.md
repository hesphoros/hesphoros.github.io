# Template Parameter Pack

# 1. 为什么需要参数包？

普通模板：

```cpp
template<typename T>
void foo(T value);
```

只能接受一个类型：

```cpp
foo(10);       // T = int
foo(3.14);     // T = double
```

如果希望：

```cpp
foo(10, 3.14, "hello", 'A');
```

接受任意数量、任意类型的参数，就需要：

```cpp
template<typename... Args>
void foo(Args... args);
```

这里的：

```cpp
typename... Args
```

就是**模板参数包**。

---

# 2. `...` 到底是什么意思？

这是参数包最容易搞混的地方。

```cpp
template<typename... Args>
```

这里：

```text
Args
```

不是一个类型。

它代表：

```text
一组类型
```

例如调用：

```cpp
foo(10, 3.14, "hello");
```

编译器可能推导成：

```text
Args = {
    int,
    double,
    const char*
}
```

所以：

```cpp
template<typename... Args>
```

可以理解成：

```text
Args = T1, T2, T3, T4, ...
```

数量可以是：

```text
0
1
2
3
...
N
```

---

# 3. 函数参数包

例如：

```cpp
template<typename... Args>
void foo(Args... args)
{
}
```

调用：

```cpp
foo(1, 2.0, "hello");
```

对应：

```text
Args = int, double, const char*
args = 1, 2.0, "hello"
```

这里有两个不同的“包”：

```cpp
typename... Args
```

是**模板类型参数包**。



```cpp
Args... args
```

是**函数参数包**。

---

# 4. 参数包不能直接使用

例如：

```cpp
template<typename... Args>
void foo()
{
    std::cout << sizeof(Args...); // ❌
}
```

因为 `Args...` 是：

```text
int, double, string
```

这种“一串东西”。

你必须使用某种**展开（pack expansion）**方式。

---

# 5. `sizeof...(Args)`

这是最简单的参数包操作。

```cpp
template<typename... Args>
void foo()
{
    std::cout << sizeof...(Args);
}
```

调用：

```cpp
foo<int, double, std::string>();
```

输出：

```text
3
```

所以：

```cpp
sizeof...(Args)
```

就是：

> **获取参数包里有多少个元素。**

同样：

```cpp
sizeof...(args)
```

也可以获取函数参数数量。

---

# 6. 参数包展开

例如：

```cpp
template<typename... Args>
void foo(Args... args)
{
    bar(args...);
}
```

如果：

```text
Args = int, double, string
args = 1, 3.14, "hello"
```

那么：

```cpp
bar(args...);
```

会展开成：

```cpp
bar(1, 3.14, "hello");
```

所以：

```cpp
...
```

在这里表示：

> **把参数包展开。**

---

# 7. C++17 的 Fold Expression

假设我们想把所有参数加起来。

C++17 可以：

```cpp
template<typename... Args>
auto sum(Args... args)
{
    return (args + ...);
}
```

调用：

```cpp
auto result = sum(1, 2, 3, 4);
```

得到：

```text
10
```

这里：

```cpp
(args + ...)
```

叫：

> **Fold Expression（折叠表达式）**

实际上相当于：

```text
1 + 2 + 3 + 4
```

---

# 8. 四种 Fold Expression

这个建议记住。

### 一元右折叠

```cpp
(args + ...)
```

展开：

```text
a + (b + (c + d))
```

---

### 一元左折叠

```cpp
(... + args)
```

展开：

```text
((a + b) + c) + d
```

---

### 二元右折叠

```cpp
(args + ... + 0)
```

展开：

```text
a + (b + (c + (d + 0)))
```

---

### 二元左折叠

```cpp
(0 + ... + args)
```

展开：

```text
(((0 + a) + b) + c) + d
```

最常用的还是：

```cpp
(args + ...)
```

和：

```cpp
(... + args)
```

---

# 9. 用 Fold Expression 打印参数

非常经典：

```cpp
template<typename... Args>
void print(Args&&... args)
{
    (std::cout << ... << args);
}
```

调用：

```cpp
print(1, "hello", 3.14);
```

相当于：

```cpp
std::cout << 1
          << "hello"
          << 3.14;
```

如果想换行：

```cpp
template<typename... Args>
void print(Args&&... args)
{
    ((std::cout << args << '\n'), ...);
}
```

这里：

```cpp
((std::cout << args << '\n'), ...)
```

实际上是在对每一个 `args` 执行：

```text
cout << args << '\n'
```

---

# 10. 参数包和 `std::tuple`

例如：

```cpp
template<typename... Args>
auto make_tuple2(Args&&... args)
{
    return std::tuple<Args...>(
        std::forward<Args>(args)...
    );
}
```

调用：

```cpp
auto t = make_tuple2(
    10,
    3.14,
    "hello"
);
```

这里：

```cpp
Args...
```

就是：

```text
int, double, const char*
```

于是：

```cpp
std::tuple<Args...>
```

展开成：

```cpp
std::tuple<int, double, const char*>
```

而：

```cpp
std::forward<Args>(args)...
```

则展开成：

```cpp
std::forward<int>(arg1),
std::forward<double>(arg2),
std::forward<const char*>(arg3)
```

这就是**参数包展开**。

---

# 11. 参数包和 `index_sequence`

假设：

```cpp
using T = std::tuple<int, double, std::string>;
```

我们想遍历：

```text
0
1
2
```

可以：

```cpp
std::index_sequence<0, 1, 2>
```

然后：

```cpp
template<typename Tuple, std::size_t... I>
void foo(
    Tuple&& t,
    std::index_sequence<I...>
)
{
    (std::cout << ... << std::get<I>(t));
}
```

这里：

```cpp
std::size_t... I
```

也是一个**非类型模板参数包**。

注意：

```cpp
typename... Args
```

是：

> 类型参数包

而：

```cpp
std::size_t... I
```

是：

> 非类型参数包

---

# 12. 参数包其实有很多种

你可以把它分类成：

```text
Template Parameter Pack
│
├── 类型参数包
│      typename... Args
│
├── 非类型参数包
│      std::size_t... I
│
└── 模板参数包
       template<typename> class... Ts
```

例如：

```cpp
template<typename... Args>
```

### 类型参数包

```cpp
Args = int, double, string
```

---

```cpp
template<int... Values>
```

### 非类型参数包

```cpp
Values = 1, 2, 3, 4
```

---

```cpp
template<template<typename> class... Containers>
```

### 模板参数包

```text
Containers =
    vector,
    list,
    deque
```

这个属于更高级的模板元编程。

---

# 13. 一个非常重要的理解

你可以把：

```cpp
template<typename... Args>
```

想象成：

```text
Args
 ↓
┌───────┬────────┬──────────┐
│ int   │ double │ string   │
└───────┴────────┴──────────┘
```

而：

```cpp
Args...
```

就是：

```text
int, double, string
```

所以看到：

```cpp
template<typename... Args>
void foo(Args&&... args)
```

脑子里直接翻译：

```text
Args = 任意数量的类型
args = 对应数量的函数参数
```





