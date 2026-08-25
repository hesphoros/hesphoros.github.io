`constexpr` 是 C++ 中用来声明**“可以在编译期求值”**的关键字。

---

## 1. 最简单的理解

```cpp
constexpr int x = 10;
```

意思可以粗略理解成：

> `x` 的值必须能够在**编译期确定**。

例如：

```cpp
constexpr int a = 10;
constexpr int b = 20;
constexpr int c = a + b;
```

编译器可以直接把：

```cpp
c
```

当成：

```cpp
30
```

而不需要运行时计算。

---

## 2. `const` 和 `constexpr` 的区别

这是最容易混淆的地方。

### `const`

```cpp
const int x = 10;
```

表示：

> `x` 初始化以后不能被修改。

但**不代表它一定是编译期常量**。

例如：

```cpp
int getValue()
{
    return 10;
}

const int x = getValue();
```

这是合法的。

`x` 不能修改，但是它的值是在**运行时**通过 `getValue()` 得到的。

---

### `constexpr`

```cpp
constexpr int x = 10;
```

则要求：

> `x` 必须是一个可以作为编译期常量使用的值。

例如：

```cpp
constexpr int x = 10;

int array[x];
```

这里 `x` 可以直接用于需要编译期常量的地方。

---

## 3. `constexpr` 不只是变量

它也可以修饰函数。

```cpp
constexpr int add(int a, int b)
{
    return a + b;
}
```

那么：

```cpp
constexpr int x = add(10, 20);
```

可以在编译期计算：

```text
add(10, 20)
      ↓
    编译器
      ↓
     30
```

---

## 4. 但 `constexpr` 函数不一定只能编译期执行

这是一个非常重要的概念。

```cpp
constexpr int add(int a, int b)
{
    return a + b;
}
```

既可以：

```cpp
constexpr int x = add(1, 2);
```

也可以：

```cpp
int a = 10;
int b = 20;

int c = add(a, b);
```

第二种就是**运行时执行**。

所以：

> `constexpr` 函数 = **允许编译器在编译期执行的函数**
> 而不是“只能在编译期执行的函数”。

---

# 5. `consteval` 又是什么？

如果你要求：

> 这个函数必须在编译期执行。

那么可以使用 `consteval`：

```cpp
consteval int add(int a, int b)
{
    return a + b;
}
```

此时：

```cpp
constexpr int x = add(1, 2); // OK
```

但是：

```cpp
int a = 10;
int b = 20;

int c = add(a, b); // error
```

因为 `a`、`b` 是运行时变量。

可以简单记：

```text
const
 ↓
不能修改

constexpr
 ↓
可以在编译期计算

consteval
 ↓
必须在编译期计算
```



---

# 6. `constexpr` + `if constexpr`

你以后写模板库会经常看到：

```cpp
if constexpr (std::is_integral_v<T>)
{
    // 整数类型
}
else
{
    // 其他类型
}
```

这里的 `if constexpr` 意思是：

> **编译期决定哪条分支存在。**

例如：

```cpp
template<typename T>
void foo(T value)
{
    if constexpr (std::is_integral_v<T>)
    {
        // T 是整数
    }
    else
    {
        // T 不是整数
    }
}
```

假设：

```cpp
foo(123);
```

编译器可能直接把模板实例化成：

```cpp
// 只有整数分支
```

另一边：

```cpp
foo(3.14);
```

则：

```cpp
// 只有非整数分支
```

这对**高性能泛型库**非常重要。

---

# 7. `constexpr` 和 `consteval` 和 `constinit`

这三个建议一起记。

| 关键字         | 核心含义          |
| ----------- | ------------- |
| `const`     | 运行后不能修改       |
| `constexpr` | 可以在编译期求值      |
| `consteval` | 必须在编译期求值      |
| `constinit` | 保证静态变量进行静态初始化 |

例如：

```cpp
const int a = 10;

constexpr int b = 10;

consteval int foo()
{
    return 10;
}

constinit int c = 10;
```

---

你现在如果准备自己设计一个高性能 C++ 编译期反射系统，我建议你把这些东西理解透：

```text
constexpr
consteval
constinit
if constexpr
template
type_traits
std::integral_constant
std::bool_constant
std::array
std::string_view
std::tuple
parameter pack
fold expression
concepts
requires
```

其中尤其重要的是：

```text
constexpr
   ↓
编译期计算能力

template
   ↓
编译期类型生成

if constexpr
   ↓
编译期分支

parameter pack
   ↓
编译期处理多个成员

std::string_view
   ↓
编译期字符串信息

concepts / requires
   ↓
编译期约束

consteval
   ↓
强制编译期执行
```

最终你做的反射系统很可能就是这种结构：

```cpp
template<typename T>
consteval auto reflect()
{
    // 编译期生成 T 的反射信息
}
```

然后：

```cpp
constexpr auto info = reflect<Person>();
```

得到一个**完全由编译器生成的类型信息对象**。

这也是你前面所说的“**高性能编译期反射**”非常核心的技术路线。
