# Detection  Idiom

**Detection Idiom（检测惯用法）**是 C++ 模板元编程里一个非常经典的技巧。

一句话理解：

> **Detection Idiom 用来在编译期检测：某个类型/表达式“是否存在、是否有效”。**

例如你想问：

```cpp
struct A {
    using value_type = int;
};

struct B {
};
```

编译期判断：

```text
A 有没有 value_type？ → true
B 有没有 value_type？ → false
```

这就是 Detection Idiom 的典型用途。

---

## 1. 为什么需要它？

假设你直接写：

```cpp
template<typename T>
using value_type_t = typename T::value_type;
```

然后：

```cpp
value_type_t<A>  // OK
value_type_t<B>  // ❌ 编译错误
```

因为 `B::value_type` 根本不存在。

我们希望的是：

```cpp
has_value_type<A> // true
has_value_type<B> // false
```

而不是让编译器直接炸掉。

Detection Idiom 就是解决这个问题的。

---

# 2. C++17 的标准写法

C++17 提供了：

```cpp
std::void_t
```

所以最经典的 Detection Idiom 是：

```cpp
#include <type_traits>

template<typename T, typename = void>
struct has_value_type : std::false_type {};

template<typename T>
struct has_value_type<T, std::void_t<typename T::value_type>>
    : std::true_type {};
```

然后：

```cpp
struct A {
    using value_type = int;
};

struct B {
};

static_assert(has_value_type<A>::value);
static_assert(!has_value_type<B>::value);
```

也可以：

```cpp
static_assert(has_value_type_v<A>);
static_assert(!has_value_type_v<B>);
```

---

# 3. 这里最关键的是 `void_t`

先看：

```cpp
std::void_t<typename T::value_type>
```

如果：

```cpp
T = A
```

那么：

```cpp
typename A::value_type
```

存在：

```cpp
std::void_t<int>
```

结果：

```cpp
void
```

于是匹配：

```cpp
template<typename T>
struct has_value_type<T, std::void_t<typename T::value_type>>
```

得到：

```cpp
true_type
```

---

如果：

```cpp
T = B
```

那么：

```cpp
typename B::value_type
```

不存在。

这里不会直接导致整个程序编译失败。

而是发生：

> **SFINAE**

于是这个特化被丢弃：

```cpp
template<typename T>
struct has_value_type<T, std::void_t<typename T::value_type>>
```

匹配失败。

最后退回：

```cpp
template<typename T, typename = void>
struct has_value_type : std::false_type {};
```

所以：

```text
A
 ↓
value_type 存在
 ↓
void_t 成功
 ↓
true_type

B
 ↓
value_type 不存在
 ↓
void_t 替换失败
 ↓
SFINAE
 ↓
主模板
 ↓
false_type
```

这就是 Detection Idiom 的核心。

---

# 4. 它不只能检测类型

这才是 Detection Idiom 真正强大的地方。

比如检测：

```cpp
T::foo()
```

是否存在。

```cpp
template<typename T, typename = void>
struct has_foo : std::false_type {};

template<typename T>
struct has_foo<T, std::void_t<
    decltype(std::declval<T>().foo())
>> : std::true_type {};
```

测试：

```cpp
struct A {
    void foo();
};

struct B {
};

static_assert(has_foo<A>::value);
static_assert(!has_foo<B>::value);
```

这里你应该能看到你最近学的几个知识点全部串起来了：

```text
Detection Idiom
       │
       ├── std::void_t
       │
       ├── SFINAE
       │
       ├── decltype
       │
       └── std::declval
```

---

# 5. 检测表达式

例如我们想检测：

```cpp
a + b
```

是否有效。

可以写：

```cpp
template<typename T, typename U, typename = void>
struct has_add : std::false_type {};

template<typename T, typename U>
struct has_add<T, U, std::void_t<
    decltype(std::declval<T>() + std::declval<U>())
>> : std::true_type {};
```

然后：

```cpp
static_assert(has_add<int, double>::value);
static_assert(has_add<std::string, std::string>::value);
```

甚至：

```cpp
struct X {};

static_assert(!has_add<X, X>::value);
```

因为：

```cpp
std::declval<X>() + std::declval<X>()
```

无效。

---

# 6. C++17 标准 Detection Idiom

C++17 实际上**没有**把完整的 Detection Idiom 做成：

```cpp
std::is_detected
```

这种标准 API。

通常自己实现。

一个比较完整的实现是：

```cpp
template<typename Default, typename AlwaysVoid,
         template<typename...> class Op,
         typename... Args>
struct detector {
    using value_t = std::false_type;
    using type = Default;
};

template<typename Default,
         template<typename...> class Op,
         typename... Args>
struct detector<
    Default,
    std::void_t<Op<Args...>>,
    Op,
    Args...
> {
    using value_t = std::true_type;
    using type = Op<Args...>;
};
```

然后：

```cpp
struct nonesuch {
    nonesuch() = delete;
    ~nonesuch() = delete;
};

template<template<typename...> class Op, typename... Args>
using is_detected =
    typename detector<nonesuch, void, Op, Args...>::value_t;

template<template<typename...> class Op, typename... Args>
using detected_t =
    typename detector<nonesuch, void, Op, Args...>::type;
```

定义一个要检测的操作：

```cpp
template<typename T>
using value_type_t = typename T::value_type;
```

然后：

```cpp
struct A {
    using value_type = int;
};

struct B {
};

static_assert(is_detected<value_type_t, A>::value);
static_assert(!is_detected<value_type_t, B>::value);
```

甚至可以拿到检测结果的类型：

```cpp
using T = detected_t<value_type_t, A>;

static_assert(std::is_same_v<T, int>);
```

---

# 7. 为什么叫 "Idiom"？

这里的 **Idiom** 不要理解成一个具体的 C++ 关键字。

它更接近：

> **一种经过大量实践形成的模板编程模式/惯用写法。**

类似：

```text
RAII
PImpl
CRTP
SFINAE
Detection Idiom
Type Erasure
```

都是 C++ 社区里非常常见的 programming idiom / technique。

---

# 8. 和 C++20 Concepts 的关系

这个非常重要。

Detection Idiom 是 **C++17 时代解决“类型是否满足某种条件”问题的经典方案**。

C++20 有了 Concepts 后，很多 Detection Idiom 可以直接写成：

```cpp
template<typename T>
concept HasValueType =
    requires {
        typename T::value_type;
    };
```

然后：

```cpp
template<HasValueType T>
void foo(T);
```

相比：

```cpp
std::void_t
SFINAE
partial specialization
```

明显更加直观。

所以可以把它们的关系理解成：

```text
C++11
 │
 └── SFINAE
       │
       ↓
C++17
 │
 └── void_t + Detection Idiom
       │
       ↓
C++20
 │
 └── Concepts / requires
```

---

