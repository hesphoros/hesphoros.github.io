# `decltype` 详细知识点

`decltype` 是 C++ 中非常重要的**类型推导工具**。

> **`decltype(expr)`：告诉编译器“表达式 `expr` 的类型是什么”，但它有一套非常特殊的规则。**

---

# 1. 基本语法

```cpp
decltype(expression)
```

例如：

```cpp
int x = 10;

decltype(x) y = 20;
```

等价于：

```cpp
int y = 20;
```

因为：

```cpp
decltype(x)
```

得到：

```cpp
int
```

---

# 2. `decltype` 和 `auto` 的核心区别

这是最重要的知识点之一。

假设：

```cpp
int x = 10;
int& ref = x;
const int cx = 20;
```

### `auto`

```cpp
auto a = x;
auto b = ref;
auto c = cx;
```

得到：

```cpp
a -> int
b -> int
c -> int
```

`auto` 会进行类似模板参数推导的[类型退化(Type Decay)](类型退化(Type%20Decay).md)

---

### `decltype`

```cpp
decltype(x)   // int
decltype(ref) // int&
decltype(cx)  // const int
```

所以：

|表达式|`decltype`|
|---|---|
|`x`|`int`|
|`ref`|`int&`|
|`cx`|`const int`|

这也是为什么模板元编程中经常使用：

```cpp
decltype(...)
```

而不是：

```cpp
auto
```

---

# 3. 最特殊的规则：未加括号的变量名

`decltype` 有一个非常特殊的规则：

如果表达式是：

```cpp
decltype(unparenthesized-id-expression)
```

也就是**没有括号包起来的变量名、函数名等**，那么 `decltype` 直接返回它的**声明类型**。

例如：

```cpp
int x = 10;

decltype(x)
```

得到：

```cpp
int
```

---

```cpp
const int x = 10;

decltype(x)
```

得到：

```cpp
const int
```

---

```cpp
int& ref = x;

decltype(ref)
```

得到：

```cpp
int&
```

这里返回的是变量 `ref` **本身的声明类型**。

---

# 4. 加括号以后完全不同

这是 `decltype` 最容易踩坑的地方。

```cpp
int x = 10;

decltype(x)    // int
decltype((x))  // int&
```

为什么？

因为：

```cpp
x
```

是特殊的“未加括号的变量名”。

而：

```cpp
(x)
```

已经是一个普通表达式。

普通表达式的 `decltype` 规则取决于**值类别（value category）**。

---

# 5. `decltype` 对普通表达式的规则

对于不是特殊变量名的表达式：

### 如果表达式是 `lvalue`

得到：

```cpp
T&
```

### 如果表达式是 `xvalue`

得到：

```cpp
T&&
```

### 如果表达式是 `prvalue`

得到：

```cpp
T
```

可以记成：

```text
lvalue  → T&
xvalue  → T&&
prvalue → T
```

这是理解 `decltype` 的核心。

---

# 6. 什么是 lvalue / xvalue / prvalue？

C++ 中表达式有值类别。

简单理解：

### lvalue

代表一个具有身份、可以持续存在的对象。

例如：

```cpp
int x;

x
```

`x` 是 lvalue。

所以：

```cpp
decltype((x))
```

得到：

```cpp
int&
```

---

### prvalue
provisional value

产生一个临时值。

例如：

```cpp
10
```

所以：

```cpp
decltype(10)
```

得到：

```cpp
int
```

---

### xvalue

代表一个即将被移动/复用资源的对象。

典型：

```cpp
std::move(x)
```

所以：

```cpp
decltype(std::move(x))
```

得到：

```cpp
int&&
```

---

# 7. 一个非常重要的表格

```cpp
int x = 10;
```

| 写法                       | 类型      |     |
| ------------------------ | ------- | --- |
| `decltype(x)`            | `int`   |     |
| `decltype((x))`          | `int&`  |     |
| `decltype(10)`           | `int`   |     |
| `decltype(std::move(x))` | `int&&` |     |

其中：

```cpp
decltype(x)
```

和：

```cpp
decltype((x))
```

是完全不同的。

---

# 8. `decltype` 不会执行表达式

这是另一个非常重要的特点。

```cpp
int foo();

decltype(foo())
```

不会调用 `foo()`。

它只是分析：

```cpp
foo()
```

这个表达式的类型。

例如：

```cpp
decltype(std::cout << "hello")
```

不会真的输出：

```text
hello
```

---

所以可以理解为：

```cpp
decltype(expression)
```

是在**编译期检查表达式类型**。

---

# 9. `decltype` 可以作用于函数调用

例如：

```cpp
int add(int a, int b);

decltype(add(1, 2))
```

得到：

```cpp
int
```

因为：

```cpp
add(1, 2)
```

的返回类型是：

```cpp
int
```

---

这在模板中非常常见。

例如：

```cpp
template <typename F, typename... Args>
decltype(auto) invoke(F&& f, Args&&... args)
{
    return f(std::forward<Args>(args)...);
}
```

这里：

```cpp
decltype(auto)
```

可以准确保留函数调用的返回类型。

---

# 10. `decltype` 获取函数返回类型

例如：

```cpp
int foo(double);
```

可以：

```cpp
decltype(foo(1.0))
```

得到：

```cpp
int
```

也可以使用：

```cpp
std::invoke_result_t<decltype(foo), double>
```

这就是 `type_traits` 与 `decltype` 经常结合的地方。

---

# 11. `decltype` 获取成员变量类型

```cpp
struct Person
{
    int age;
    std::string name;
};
```

可以：

```cpp
Person p;

decltype(p.age)
```

得到：

```cpp
int
```

---

如果：

```cpp
const Person p{};
```

注意：

```cpp
decltype(p.age)
```

依然与 `age` 的声明类型相关。

而：

```cpp
decltype((p.age))
```

则会根据表达式值类别推导。

如果 `p` 是 const：

```cpp
decltype((p.age))
```

得到：

```cpp
const int&
```

这就是前面“变量名特殊规则”和“普通表达式规则”的区别。

---

# 12. `decltype` 获取成员函数返回值

例如：

```cpp
struct Foo
{
    std::string name();

    int size() const;
};
```

可以：

```cpp
decltype(std::declval<Foo>().name())
```

得到：

```cpp
std::string
```

以及：

```cpp
decltype(std::declval<const Foo>().size())
```

得到：

```cpp
int
```

这就是现代 C++ 模板元编程非常经典的写法。

---

# 13. `std::declval` + `decltype`

你学习模板元编程时会大量看到：

```cpp
decltype(std::declval<T>().foo())
```

例如：

```cpp
template <typename T>
using foo_result_t = decltype(std::declval<T>().foo());
```

意思是：

> **获取类型 `T` 调用 `foo()` 后的返回类型。**

例如：

```cpp
struct Test
{
    double foo();
};

using result = foo_result_t<Test>;
```

得到：

```cpp
double
```

注意：

```cpp
std::declval<T>()
```

并不会真的创建对象。

它只是用于**编译期类型推导**。

---

# 14. `decltype(auto)`

这是 `decltype` 非常重要的应用。

普通：

```cpp
auto foo()
{
    int x = 10;
    return x;
}
```

返回：

```cpp
int
```

---

而：

```cpp
decltype(auto) foo()
{
    int x = 10;
    return x;
}
```

这里 `return x` 的类型通过：

```cpp
decltype(x)
```

推导。

所以：

```cpp
int
```

---

但：

```cpp
decltype(auto) foo()
{
    int x = 10;
    return (x);
}
```

这里：

```cpp
decltype((x))
```

是：

```cpp
int&
```

所以函数试图返回：

```cpp
int&
```

但 `x` 是局部变量，会产生悬空引用。

这是一个非常经典的坑。

---

# 15. `auto` vs `decltype(auto)`

这是模板编程必须掌握的区别。

```cpp
int x = 10;
int& get()
{
    return x;
}
```

### 使用 `auto`

```cpp
auto foo()
{
    return get();
}
```

返回：

```cpp
int
```

引用被丢掉。

---

### 使用 `decltype(auto)`

```cpp
decltype(auto) foo()
{
    return get();
}
```

得到：

```cpp
int&
```

因为：

```cpp
decltype(get())
```

是：

```cpp
int&
```

---

因此：

```text
auto
    ↓
进行类型推导 + 退化

decltype(auto)
    ↓
按照 decltype 规则推导
```

---

# 16. `decltype(auto)` 在泛型包装器中的意义

例如：

```cpp
template <typename F>
decltype(auto) wrapper(F&& f)
{
    return f();
}
```

假设：

```cpp
int x = 10;

auto f = [&]() -> int& {
    return x;
};
```

如果：

```cpp
template <typename F>
auto wrapper(F&& f)
{
    return f();
}
```

返回：

```cpp
int
```

引用丢失。

而：

```cpp
template <typename F>
decltype(auto) wrapper(F&& f)
{
    return f();
}
```

可以保留：

```cpp
int&
```

这对于泛型库非常重要。

---

# 17. `decltype` 与 `const`

例如：

```cpp
const int x = 10;
```

```cpp
decltype(x)
```

是：

```cpp
const int
```

而：

```cpp
auto y = x;
```

则：

```cpp
int
```

所以：

```cpp
decltype(x) // const int
auto y = x;  // int
```

---

# 18. `decltype` 与引用

```cpp
int x = 10;

int& ref = x;
int&& rref = 10;
```

那么：

```cpp
decltype(ref)
```

是：

```cpp
int&
```

而：

```cpp
decltype(rref)
```

是：

```cpp
int&&
```

注意：

```cpp
decltype(ref)
```

不是：

```cpp
decltype((ref))
```

后者是：

```cpp
int&
```

因为 `ref` 表达式本身是 lvalue。

---

# 19. 一个很容易混淆的例子

```cpp
int&& rref = 10;
```

很多人会认为：

```cpp
decltype((rref))
```

应该是：

```cpp
int&&
```

实际上：

```cpp
decltype(rref)   // int&&
decltype((rref)) // int&
```

为什么？

因为：

```cpp
rref
```

这个变量的**声明类型**是：

```cpp
int&&
```

但是：

> **一个有名字的右值引用变量，在表达式中本身是 lvalue。**

因此：

```cpp
(rref)
```

是 lvalue。

所以：

```cpp
decltype((rref))
```

得到：

```cpp
int&
```

这是 C++ 模板编程中非常重要的知识。

---

# 20. `decltype` 与成员函数

例如：

```cpp
struct Foo
{
    int get();
};
```

可以：

```cpp
Foo foo;

decltype(foo.get())
```

得到：

```cpp
int
```

但是如果：

```cpp
struct Foo
{
    int& get();
};
```

那么：

```cpp
decltype(foo.get())
```

就是：

```cpp
int&
```

`decltype` 会准确保留返回值类型。

---

# 21. `decltype` 与成员函数指针


例如：

```cpp
struct Test
{
    int foo(double);
};
```

可以：

```cpp
auto ptr = &Test::foo;
```

此时：

```cpp
decltype(ptr)
```

得到：

```cpp
int (Test::*)(double)
```

比如

```cpp
template <auto ptr>
auto* type_ensure()
{
    using traits = member_fn_traits<decltype(ptr)>;
}
```

就是利用：

```cpp
decltype(ptr)
```

取得模板参数 `ptr` 的**精确类型**。

---

# 22. `decltype` + 模板参数

例如：

```cpp
template <typename T>
void foo(T value)
{
    using type = decltype(value);
}
```

如果：

```cpp
foo(10);
```

那么：

```cpp
T = int
```

因此：

```cpp
decltype(value)
```

是：

```cpp
int
```

---

再例如：

```cpp
template <typename T>
void foo(T& value)
{
    using type = decltype(value);
}
```

那么：

```cpp
int x;
foo(x);
```

得到：

```cpp
decltype(value) == int&
```

---

# 23. `decltype` + SFINAE

这是非常重要的模板元编程应用。

例如我们想判断：

> `T` 有没有 `foo()`。

可以：

```cpp
template <typename T>
auto test(int)
    -> decltype(std::declval<T>().foo(), std::true_type{});
```

如果：

```cpp
T::foo()
```

存在：

```cpp
std::declval<T>().foo()
```

合法。

于是：

```cpp
decltype(...)
```

能够成功推导。

如果不存在：

```cpp
T::foo()
```

表达式非法。

模板发生 SFINAE。

---

所以经典的：

```cpp
decltype(std::declval<T>().foo())
```

背后其实连接了三个重要知识点：

```text
std::declval
      ↓
构造一个“假想表达式”
      ↓
decltype
      ↓
检查表达式类型是否合法
      ↓
SFINAE
```

这也是早期 C++ 模板元编程的重要基础。

---

# 24. C++20 Concepts 中仍然大量使用这个思想

现代 C++ 中可能不需要手写复杂 SFINAE，但思想依然存在。

例如：

```cpp
template <typename T>
concept HasFoo = requires(T t)
{
    t.foo();
};
```

这里的：

```cpp
t.foo()
```

本质上仍然是在做：

> **编译期检查某个表达式是否有效。**

而 `decltype` 是早期实现这种能力的核心工具之一。

---

# 25. `decltype` + `std::is_same`

这是学习 `decltype` 时非常推荐的练习。

```cpp
#include <type_traits>

int x = 10;

static_assert(
    std::is_same_v<decltype(x), int>
);
```

再看：

```cpp
static_assert(
    std::is_same_v<decltype((x)), int&>
);
```

这可以帮助你真正理解：

```text
decltype(x)
```

和：

```text
decltype((x))
```

为什么不同。

---

# 26. `decltype` 规则

可以直接记这个：

```text
decltype(expr)
```

首先问：

### 第一步：是不是未加括号的变量名？

如果是：

```cpp
decltype(x)
```

那么：

> **直接取 x 的声明类型。**

---

如果不是：

### 第二步：看值类别

```text
lvalue
   ↓
T&

xvalue
   ↓
T&&

prvalue
   ↓
T
```

---

所以：

```cpp
int x;

decltype(x)      // int
decltype((x))    // int&
decltype(10)     // int
decltype(std::move(x)) // int&&
```
