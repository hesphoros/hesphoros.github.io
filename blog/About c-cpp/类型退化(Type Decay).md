# Type Decay

类型退化(ype Decay)是 C++ 类型推导里非常重要的一个概念。

你可以先把它理解成：

> 类型退化 = 在某些类型推导场景中，C++ 会主动丢掉 `const`、引用等信息，把一个“复杂类型”转换成一个更适合按值传递的“普通值类型”。

这也是为什么：

```cpp
auto a = x;
```

和：

```cpp
decltype(x) a = x;
```

经常得到不同的类型。

---

# 1. 最直观的例子：引用退化

```cpp
int x = 10;
int& ref = x;
```

现在：

```cpp
auto a = ref;
```

`ref` 的类型是：

```cpp
int&
```

但是：

```cpp
a
```

的类型是：

```cpp
int
```

也就是说：

```text
int&
  ↓ 类型退化
int
```

引用被去掉了。

---

# 2. `const` 也会退化

```cpp
const int x = 10;

auto a = x;
```

这里：

```cpp
decltype(x) // const int
```

但是：

```cpp
decltype(a) // int
```

发生了：

```text
const int
    ↓
  int
```

`const` 顶层限定被去掉了。

---

# 3. 为什么 C++ 要这么做？

因为：

```cpp
auto a = x;
```

本质上是在说：

> **我要创建一个新的对象，并把 `x` 的值复制给它。**

例如：

```cpp
int x = 10;

auto a = x;
```

内存关系是：

```text
x
┌───────┐
│  10   │
└───────┘
   ↓ copy

a
┌───────┐
│  10   │
└───────┘
```

`a` 是一个**新的对象**。

所以没必要让：

```cpp
a
```

保持 `x` 的引用属性。

---

# 4. 引用退化

最常见的就是：

```cpp
int x = 10;
int& r = x;

auto a = r;
```

虽然：

```cpp
decltype(r) // int&
```

但是：

```cpp
decltype(a) // int
```

因为 `auto` 推导时：

```text
int&
 ↓
int
```

---

# 5. `const` 退化

```cpp
const int x = 10;

auto a = x;
```

结果：

```cpp
decltype(x) // const int
decltype(a) // int
```

---

# 6. `const` 引用

这个更加典型：

```cpp
const int x = 10;
const int& r = x;

auto a = r;
```

`r`：

```cpp
const int&
```

经过 `auto` 推导：

```text
const int&
    ↓
  const int
    ↓
    int
```

最终：

```cpp
decltype(a) // int
```

---

# 7. 数组退化

类型退化不只是引用和 `const`。

数组也是非常重要的一种。

```cpp
int arr[5];
```

数组真正的类型是：

```cpp
int[5]
```

但是：

```cpp
auto x = arr;
```

得到的不是：

```cpp
int[5]
```

而是：

```cpp
int*
```

发生：

```text
int[5]
  ↓
int*
```

这就是**数组到指针的退化**。

---

# 8. 函数也会退化

例如：

```cpp
void foo();
```

函数类型：

```cpp
void()
```

但是：

```cpp
auto f = foo;
```

这里 `f` 的类型不是函数类型，而是：

```cpp
void(*)()
```

也就是：

```text
函数类型
   ↓
函数指针
```

---

# 9. 所以“类型退化”主要包含什么？

经典的 C++ 类型退化可以记成：

```text
引用
  ↓
去掉引用

const / volatile
  ↓
去掉顶层 cv

数组
  ↓
指针

函数
  ↓
函数指针
```

例如：

```text
int&              → int
const int&        → int
volatile int      → int

int[10]            → int*
void()             → void(*)()
```

---

# 10. `auto` 为什么会发生退化？

因为：

```cpp
auto
```

的推导规则和模板参数推导非常接近。

例如：

```cpp
template <typename T>
void foo(T value);
```

调用：

```cpp
int x = 10;
int& r = x;

foo(r);
```

推导出来：

```cpp
T = int
```

而不是：

```cpp
T = int&
```

因为这里的 `T` 是**按值推导**。

所以：

```cpp
auto a = r;
```

和：

```cpp
template <typename T>
void foo(T);

foo(r);
```

在类型推导上具有非常相似的行为。

---

# 11. 但是 `auto&` 不会这样

例如：

```cpp
int x = 10;

auto& r = x;
```

这里：

```cpp
decltype(r)
```

是：

```cpp
int&
```

因为你明确告诉编译器：

> 我要一个引用。

所以：

```text
auto
 ↓
通常丢掉引用

auto&
 ↓
保留引用

auto&&
 ↓
根据值类别进行转发引用推导
```

---

# 12. `auto`、`auto&`、`decltype` 对比

```cpp
int x = 10;
const int cx = 20;
int& ref = x;
```

| 写法            | 类型         |
| --------------- | ------------ |
| `auto a = x`    | `int`        |
| `auto b = ref`  | `int`        |
| `auto c = cx`   | `int`        |
| `auto& d = x`   | `int&`       |
| `auto& e = cx`  | `const int&` |
| `decltype(x)`   | `int`        |
| `decltype(ref)` | `int&`       |
| `decltype(cx)`  | `const int`  |

所以你可以看到：

```text
auto
```

更倾向于：

> **我要这个值本身。**

而：

```text
decltype
```

更倾向于：

> **我要这个表达式/实体的精确类型。**

---

# 13. `std::decay`

C++ 标准库甚至直接提供了一个工具：

```cpp
std::decay_t<T>
```

它就是专门模拟这种“退化”的。

例如：

```cpp
using A = std::decay_t<int&>;
```

得到：

```cpp
int
```

---

```cpp
using B = std::decay_t<const int&>;
```

得到：

```cpp
int
```

---

数组：

```cpp
using C = std::decay_t<int[10]>;
```

得到：

```cpp
int*
```

---

函数：

```cpp
using D = std::decay_t<void()>;
```

得到：

```cpp
void(*)()
```

---

# 14. `std::remove_reference` 和 `std::decay` 不一样

这是学习 `type_traits` 时很容易混淆的。

```cpp
using A = std::remove_reference_t<const int&>;
```

结果：

```cpp
const int
```

因为它只做：

```text
const int&
     ↓
const int
```

但是：

```cpp
using B = std::decay_t<const int&>;
```

结果：

```cpp
int
```

因为 `decay` 进一步去掉了顶层 `const`。

---

可以简单理解：

```text
remove_reference
        ↓
只去引用

remove_cv
        ↓
只去 const / volatile

decay
        ↓
模拟按值传递时的类型退化
```

---

# 15. 为什么模板元编程特别关心退化？

因为模板经常需要回答：

> **我到底拿到的是 `T`、`T&`、`const T&` 还是 `T&&`？**

例如：

```cpp
template <typename T>
void foo(T value)
{
}
```

这里：

```cpp
T
```

通常已经是退化后的类型。

但：

```cpp
template <typename T>
void foo(T&& value)
{
}
```

情况就完全不同。

---

# 16. 一个非常重要的区别

你刚刚问的是 `decltype`，所以这里尤其要注意：

```cpp
int x = 10;

auto a = x;
```

得到：

```cpp
int
```

但：

```cpp
decltype(x) b = x;
```

得到：

```cpp
int
```

如果：

```cpp
int& ref = x;
```

那么：

```cpp
auto a = ref;
```

得到：

```cpp
int
```

而：

```cpp
decltype(ref) b = ref;
```

得到：

```cpp
int&
```

所以：

```text
auto
    → 推导 + 退化

decltype
    → 精确分析类型
```

这是你理解 `decltype` 最关键的背景知识之一。

---

# 17. 还有一个非常容易误解的地方

严格来说，“类型退化”不是简单地：

> “C++ 总是把引用和 const 去掉。”

而是：

> **在特定的类型推导/类型转换规则中，发生的一组标准化类型转换。**

尤其是：

```cpp
std::decay_t<T>
```

它的定义可以粗略理解为：

```text
T
│
├─ 如果是引用 → 去引用
│
├─ 如果是数组 → 转指针
│
├─ 如果是函数 → 转函数指针
│
└─ 去掉顶层 cv
```

所以它比简单的：

```cpp
remove_reference
```

更强。
