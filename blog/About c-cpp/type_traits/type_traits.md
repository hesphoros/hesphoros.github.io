# C++ `type_traits` 知识点体系

## 1. type_traits 的核心概念

Trait:特征

### 1.1 什么是 Type Trait

Trait 可以理解为：

> **在编译期描述、判断或者修改一个类型的元信息。**

例如：

```cpp
std::is_integral_v<int>        // true
std::is_integral_v<double>     // false

std::is_same_v<int, int>       // true
std::is_same_v<int, float>     // false
```

这些结果都在**编译期**确定。

---

### 1.2 `std::true_type` / `std::false_type`

很多 Trait 的基础：

```cpp
std::true_type
std::false_type
```

本质上类似：

```cpp
using true_type = std::integral_constant<bool, true>;
using false_type = std::integral_constant<bool, false>;
```

例如：

```cpp
std::is_integral<int>
```

实际上是一个类型：

```cpp
std::true_type
```

所以：

```cpp
std::is_integral<int>::value
```

得到：

```cpp
true
```

---

## 2. `std::integral_constant`

这是理解 `<type_traits>` 非常重要的基础。

```cpp
template<class T, T v>
struct integral_constant;
```

例如：

```cpp
using Five = std::integral_constant<int, 5>;

static_assert(Five::value == 5);
```

它同时提供：

```cpp
Five::value
Five::value_type
Five::type
```

C++17 还可以使用：

```cpp
std::bool_constant<true>
```

因此：

```cpp
std::true_type
std::false_type
```

都是建立在这个机制之上的。

---

# 3. 类型相等判断

## 3.1 `std::is_same`

判断两个类型是否完全相同：

```cpp
std::is_same<T, U>
```

C++17：

```cpp
std::is_same_v<T, U>
```

例如：

```cpp
static_assert(std::is_same_v<int, int>);
static_assert(!std::is_same_v<int, float>);
```

这是模板元编程中最常见的 Trait 之一。

---

# 4. 基础类型分类

## 4.1 整数类型

```cpp
std::is_integral_v<T>
```

判断：

```cpp
bool
char
short
int
long
long long
```

以及对应的 `unsigned` 类型。

---

## 4.2 浮点类型

```cpp
std::is_floating_point_v<T>
```

例如：

```cpp
float
double
long double
```

---

## 4.3 算术类型

```cpp
std::is_arithmetic_v<T>
```

相当于：

```text
整数类型
+
浮点类型
```

---

## 4.4 基础分类

还需要掌握：

```cpp
std::is_void_v<T>

std::is_null_pointer_v<T>

std::is_enum_v<T>

std::is_union_v<T>

std::is_class_v<T>

std::is_function_v<T>
```

例如：

```cpp
static_assert(std::is_class_v<MyClass>);
static_assert(std::is_enum_v<MyEnum>);
```

---

# 5. 指针与引用判断

非常重要。

## 5.1 指针

```cpp
std::is_pointer_v<T>
```

例如：

```cpp
std::is_pointer_v<int*>       // true
std::is_pointer_v<int>        // false
```

---

## 5.2 左值引用

```cpp
std::is_lvalue_reference_v<T>
```

---

## 5.3 右值引用

```cpp
std::is_rvalue_reference_v<T>
```

---

## 5.4 任意引用

```cpp
std::is_reference_v<T>
```

相当于：

```text
lvalue reference
+
rvalue reference
```

---

## 5.5 成员指针

```cpp
std::is_member_pointer_v<T>
```

例如：

```cpp
struct Foo
{
    int value;
};

using T = int Foo::*;

static_assert(std::is_member_pointer_v<T>);
```

---

# 6. `const` / `volatile` 判断

## 6.1 `is_const`

```cpp
std::is_const_v<const int>
```

结果：

```cpp
true
```

---

## 6.2 `is_volatile`

```cpp
std::is_volatile_v<volatile int>
```

---

## 6.3 两者一起

```cpp
std::is_const_v<const volatile int>
std::is_volatile_v<const volatile int>
```

---

# 7. 类型属性判断

这一部分对于**编译期序列化 / 反射 / 类型系统**尤其重要。

## 7.1 默认构造

```cpp
std::is_default_constructible_v<T>
```

---

## 7.2 拷贝构造

```cpp
std::is_copy_constructible_v<T>
```

---

## 7.3 移动构造

```cpp
std::is_move_constructible_v<T>
```

---

## 7.4 拷贝赋值

```cpp
std::is_copy_assignable_v<T>
```

---

## 7.5 移动赋值

```cpp
std::is_move_assignable_v<T>
```

---

## 7.6 析构

```cpp
std::is_destructible_v<T>
```

---

# 8. 构造 / 转换能力

## 8.1 `is_constructible`

判断：

```cpp
T
```

是否能够通过指定参数构造。

```cpp
std::is_constructible_v<T, Args...>
```

例如：

```cpp
struct Foo
{
    Foo(int, double);
};

static_assert(
    std::is_constructible_v<Foo, int, double>
);
```

---

## 8.2 `is_convertible`

判断两个类型能否隐式转换：

```cpp
std::is_convertible_v<From, To>
```

例如：

```cpp
std::is_convertible_v<int, double> // true
```

---

## 8.3 `is_nothrow_constructible`

判断构造是否 `noexcept`：

```cpp
std::is_nothrow_constructible_v<T, Args...>
```

这在泛型代码、容器和高性能代码里很重要。

---

# 9. Trivial 类型判断

Trivial：琐碎的

这是深入 C++ 类型系统时非常重要的一组。

```cpp
std::is_trivial_v<T>
```

不过需要特别注意：

> `std::is_trivial` 在现代 C++ 中已经属于逐渐被淘汰的旧式概念。

更应该掌握：

```cpp
std::is_trivially_copyable_v<T>

std::is_trivially_constructible_v<T>

std::is_trivially_destructible_v<T>

std::is_trivially_copy_constructible_v<T>

std::is_trivially_move_constructible_v<T>

std::is_trivially_copy_assignable_v<T>

std::is_trivially_move_assignable_v<T>
```

例如：

```cpp
static_assert(std::is_trivially_copyable_v<int>);
```

---

# 10. `standard_layout`

```cpp
std::is_standard_layout_v<T>
```

用于判断一个类型是否满足 C++ 的 standard-layout 要求。

这个概念和：

```text
ABI
内存布局
C 兼容结构
offsetof
二进制序列化
```

等问题关系很大。

---

# 11. `is_empty`

判断一个类型是否为空类：

```cpp
std::is_empty_v<T>
```

例如：

```cpp
struct Empty {};

static_assert(std::is_empty_v<Empty>);
```

---

# 12. `is_polymorphic`

判断类型是否具有多态性：

```cpp
std::is_polymorphic_v<T>
```

例如：

```cpp
struct Base
{
    virtual ~Base() = default;
};
```

---

# 13. `is_final`

判断类是否使用：

```cpp
final
```

例如：

```cpp
struct Foo final {};
```

```cpp
std::is_final_v<Foo>
```

---

# 14. `is_abstract`

判断抽象类：

```cpp
std::is_abstract_v<T>
```

---

# 15. 类型关系

## 15.1 `is_base_of`

判断：

```text
Base 是否为 Derived 的基类
```

```cpp
std::is_base_of_v<Base, Derived>
```

---

## 15.2 `is_convertible`

经常和：

```cpp
std::is_base_of
```

一起使用。

例如：

```cpp
std::is_convertible_v<Derived*, Base*>
```

---

# 16. 类型变换 —— `<type_traits>` 最核心的部分之一

如果说前面是：

> **Type Query**

那么下面就是：

> **Type Transformation**

---

# 17. `remove_const`

```cpp
std::remove_const_t<const int>
```

得到：

```cpp
int
```

---

# 18. `remove_volatile`

```cpp
std::remove_volatile_t<volatile int>
```

---

# 19. `remove_cv`

同时删除：

```text
const
volatile
```

```cpp
std::remove_cv_t<const volatile int>
```

得到：

```cpp
int
```

---

# 20. `add_const`

```cpp
std::add_const_t<int>
```

得到：

```cpp
const int
```

---

# 21. `add_volatile`

```cpp
std::add_volatile_t<int>
```

---

# 22. `add_cv`

```cpp
std::add_cv_t<int>
```

---

# 23. `remove_reference`

非常重要：

```cpp
std::remove_reference_t<T>
```

例如：

```cpp
std::remove_reference_t<int&>   // int
std::remove_reference_t<int&&>  // int
```

---

# 24. `add_lvalue_reference`

```cpp
std::add_lvalue_reference_t<int>
```

得到：

```cpp
int&
```

---

# 25. `add_rvalue_reference`

```cpp
std::add_rvalue_reference_t<int>
```

得到：

```cpp
int&&
```

---

# 26. `remove_pointer`

```cpp
std::remove_pointer_t<int*>
```

得到：

```cpp
int
```

---

# 27. `add_pointer`

```cpp
std::add_pointer_t<int>
```

得到：

```cpp
int*
```

---

# 28. `remove_cvref`

这是现代 C++ 非常重要的一个：

```cpp
std::remove_cvref_t<T>
```

相当于：

```cpp
remove_cv<
    remove_reference_t<T>
>
```

例如：

```cpp
std::remove_cvref_t<const Foo&>
```

最终：

```cpp
Foo
```

对于现代泛型编程非常常用。

---

# 29. `decay`

```cpp
std::decay_t<T>
```

它模拟类似函数传参时发生的类型退化。比如数组会衰变为指向其首元素的指针，函数会衰变为指向该函数的指针。顶层的const和volaitile修饰符会被移除

例如：

```cpp
const int&
```

经过：

```cpp
std::decay_t<const int&>
```

变成：

```cpp
int
```

---

# 30. `common_type`

寻找多个类型的公共类型：

```cpp
std::common_type_t<T, U>
```

例如：

```cpp
std::common_type_t<int, double>
```

得到：

```cpp
double
```

多个类型也可以：

```cpp
std::common_type_t<int, float, double>
```

---

# 31. `common_reference`

现代 C++ 泛型编程中非常重要：

```cpp
std::common_reference_t<T, U>
```

它和：

```cpp
common_type
```

不同。

`common_type` 更关注：

> 值类型之间的公共类型。

`common_reference` 更关注：

> 引用/值类别之间能否找到公共引用类型。

Ranges 等现代 C++ 库大量使用它。

---

# 32. `underlying_type`

用于枚举：

```cpp
enum class Color : uint32_t
{
    Red,
    Green,
    Blue
};
```

可以得到底层类型：

```cpp
std::underlying_type_t<Color>
```

得到：

```cpp
uint32_t
```

对于**反射、序列化、枚举注册系统**特别有用。

---

# 33. `make_signed`

```cpp
std::make_signed_t<unsigned int>
```

得到：

```cpp
int
```

---

# 34. `make_unsigned`

```cpp
std::make_unsigned_t<int>
```

得到：

```cpp
unsigned int
```

---

# 35. `conditional`

提供成员 typedef `type`，其定义为 `T` 若 `B` 在编译时为 true，否则为 `F` 若 `B` 为 false。

若程序为 `std::conditional` 添加特化，则行为未定义

这是模板元编程中的：

> **编译期 if**

```cpp
std::conditional_t<Condition, T, F>
```

例如：

```cpp
using Type =
    std::conditional_t<
        sizeof(int) == 4,
        int,
        long long
    >;

```

---

# 36. `enable_if`

经典 [SFINAE](../SFINAE.md) 工具：

```cpp
std::enable_if_t<Condition, T>
```

例如：

```cpp
template<typename T>
std::enable_if_t<std::is_integral_v<T>>
foo(T value);
```

只有整数类型才能匹配。

不过现代 C++ 中通常更推荐：

```cpp
requires
```

或者：

```cpp
std::enable_if
```

替换成 Concepts。

---

# 37. `void_t` cpp11

### 成员常量

| 成员常量              |                                                                           |
| ----------------- | ------------------------------------------------------------------------- |
| value<br><br>[静态] | 如果 `T` 是 void 类型（可能带 cv 限定符），则为 true，否则为 false。  <br>(public static 成员常量) |

cpp17
```c
template< class T >  
constexpr bool is_void_v = is_void<T>::value;
```
这是理解 SFINAE 非常重要的工具：

```cpp
std::void_t<T...>
```

经典用途是检测某个类型是否存在成员：

```cpp
template<typename T, typename = void>
struct has_value : std::false_type {};

template<typename T>
struct has_value<T, std::void_t<typename T::value_type>>
    : std::true_type {};
```

这是：

> **Detection Idiom**

的基础。

---

# 38. `invoke_result`

判断调用某个 Callable 后得到的类型：

```cpp
std::invoke_result_t<F, Args...>
```

例如：

```cpp
auto foo(int) -> double;
```

那么：

```cpp
std::invoke_result_t<decltype(foo), int>
```

得到：

```cpp
double
```

---

# 39. `is_invocable`

判断：

```text
一个 Callable 是否可以使用指定参数调用
```

```cpp
std::is_invocable_v<F, Args...>
```

这对：

```text
lambda
函数指针
成员函数指针
function object
```

都非常有用。

---

# 40. `is_invocable_r`

不仅判断能不能调用，还判断结果是否可以转换为指定类型：

```cpp
std::is_invocable_r_v<R, F, Args...>
```

---

# 41. `is_nothrow_invocable`

判断调用是否不会抛异常：

```cpp
std::is_nothrow_invocable_v<F, Args...>
```

---

# 42. 函数类型 Traits

需要掌握：

```cpp
std::is_function_v<T>
```

以及现代 C++ 中与函数调用相关的：

```cpp
is_invocable
invoke_result
```

这部分对于实现：

```text
Delegate
Signal / Slot
Callback
Reflection
RPC
Event System
```

非常重要。

---

# 43. `is_signed` / `is_unsigned`

```cpp
std::is_signed_v<T>

std::is_unsigned_v<T>
```

例如：

```cpp
std::is_signed_v<int>           // true
std::is_unsigned_v<unsigned>    // true
```

---

# 44. `is_bounded_array` / `is_unbounded_array`

现代 C++：

```cpp
std::is_array_v<T>

std::is_bounded_array_v<T>

std::is_unbounded_array_v<T>
```

例如：

```cpp
int[10]
```

属于：

```cpp
is_bounded_array
```

而：

```cpp
int[]
```

属于：

```cpp
is_unbounded_array
```

---

# 45. `rank`

获取数组维度：

```cpp
int[3][4]
```

```cpp
std::rank_v<int[3][4]>
```

结果：

```text
2
```

---

# 46. `extent`

获取数组某一维大小：

```cpp
std::extent_v<int[3][4], 0> // 3
std::extent_v<int[3][4], 1> // 4
```

对于反射、序列化、编译期容器处理很有用。

---

# 47. `tuple_size` / `tuple_element`

严格来说它们不只属于 `<type_traits>`，但在现代类型编程中经常一起使用。

```cpp
std::tuple_size_v<T>
```

以及：

```cpp
std::tuple_element_t<I, T>
```

例如：

```cpp
std::tuple<int, double, float>
```

可以：

```cpp
std::tuple_element_t<1, T>
```

得到：

```cpp
double
```

这对：

```text
tuple
pair
array
structured binding
反射模拟
```

非常重要。

---

# 48. `type_identity`

C++20：

```cpp
std::type_identity_t<T>
```

看起来非常简单：

```cpp
std::type_identity_t<int>
```

还是：

```cpp
int
```

但它可以阻止模板参数推导。

这是理解：

> **Template Argument Deduction**

非常好的例子。

---

# 49. `is_constant_evaluated`

C++20：

```cpp
std::is_constant_evaluated()
```

判断当前代码是否正在：

```text
constant evaluation
```

环境中执行。

例如：

```cpp
constexpr int foo()
{
    if (std::is_constant_evaluated())
    {
        // 编译期
    }
    
    return 42;
}
```

---

# 50. `is_scoped_enum`

C++23：

```cpp
std::is_scoped_enum_v<T>
```

判断是不是：

```cpp
enum class
```

类型。

这对于反射系统里的 Enum Trait 很实用。

---

# 51. C++23 类型 Trait 新增内容：

```cpp
std::is_scoped_enum
```

以及：

```cpp
std::is_bounded_array
std::is_unbounded_array
```

同时重点掌握现代泛型编程逐渐从：

```cpp
enable_if
SFINAE
void_t
```

转向：

```cpp
Concepts
requires
```

---

# 52. `type_traits` 可以分成 5 大类

```text
std::type_traits
│
├── ① Type Query
│   ├── is_same
│   ├── is_integral
│   ├── is_floating_point
│   ├── is_pointer
│   ├── is_reference
│   ├── is_const
│   ├── is_enum
│   ├── is_class
│   ├── is_base_of
│   └── ...
│
├── ② Type Transformation
│   ├── remove_const
│   ├── remove_reference
│   ├── remove_pointer
│   ├── remove_cvref
│   ├── add_const
│   ├── add_pointer
│   ├── decay
│   ├── conditional
│   ├── common_type
│   ├── common_reference
│   └── ...
│
├── ③ Type Property
│   ├── is_constructible
│   ├── is_assignable
│   ├── is_destructible
│   ├── is_trivially_copyable
│   ├── is_standard_layout
│   ├── is_polymorphic
│   ├── is_empty
│   └── ...
│
├── ④ Callable Traits
│   ├── is_invocable
│   ├── is_invocable_r
│   ├── invoke_result
│   ├── is_nothrow_invocable
│   └── ...
│
└── ⑤ Template Metaprogramming
    ├── integral_constant
    ├── true_type
    ├── false_type
    ├── bool_constant
    ├── enable_if
    ├── void_t
    └── type_identity
```

---







### # TO Learn

```text
constexpr
   ↓
Template
   ↓
Type Traits
   ↓
SFINAE / Detection Idiom
   ↓
Concepts / requires
   ↓
NTTP
   ↓
Parameter Pack
   ↓
Tuple / Type List
   ↓
Compile-time Reflection
```

其中真正值得你深入源码理解的，不是 `is_integral` 这种简单 Trait，而是 **`integral_constant → SFINAE → void_t → detection idiom → type transformation → Concepts`** 这一整条演进路线。