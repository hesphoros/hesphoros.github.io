# `std::declval` 学习笔记

`std::declval` 是 C++ 模板元编程中一个非常核心、但初看很奇怪的工具。

---

# 1. `std::declval` 是什么？

定义在：

```cpp
#include <utility>
```

使用形式：

```cpp
std::declval<T>()
```

它的核心作用可以概括成一句话：

> **在编译期“假装”有一个 `T` 类型的对象，然后拿这个对象去构造表达式，从而检查表达式的类型或是否合法。**

最经典的组合：

```cpp
decltype(std::declval<T>().foo())
```

意思就是：

> **假装有一个 `T` 对象，然后调用它的 `foo()`，最后获取 `foo()` 的返回类型。**

---

# 2. 为什么需要 `std::declval`？

假设：

```cpp
struct Person
{
    std::string getName();
};
```

如果我们真的有对象：

```cpp
Person p;

decltype(p.getName())
```

很好理解：

```cpp
std::string
```

但是模板元编程里面，我们可能只有：

```cpp
template <typename T>
void test();
```

这里根本没有：

```cpp
T object;
```

因为：

- `T` 可能没有默认构造函数
  
- `T` 可能是抽象类
  
- `T` 可能无法构造
  
- 我们根本不想真的创建对象
  

这时候：

```cpp
std::declval<T>()
```

就解决了这个问题。

---

# 3. 最经典的例子

```cpp
template <typename T>
using foo_result_t =
    decltype(std::declval<T>().foo());
```

假设：

```cpp
struct Test
{
    double foo();
};
```

那么：

```cpp
using R = foo_result_t<Test>;
```

得到：

```cpp
double
```

整个过程：

```text
T
↓
std::declval<T>()
↓
假装存在一个 T 对象
↓
.std::foo()
↓
foo() 表达式
↓
decltype(...)
↓
double
```

---

# 4. `std::declval` 并不会真的创建对象

这是最重要的概念之一。

看到：

```cpp
std::declval<T>()
```

千万不要理解成：

```cpp
T{};
```

或者：

```cpp
T();
```

它不是构造对象。

例如：

```cpp
struct NonConstructible
{
    NonConstructible() = delete;

    int foo();
};
```

你不能：

```cpp
NonConstructible obj;
```

但是仍然可以：

```cpp
using R =
    decltype(std::declval<NonConstructible>().foo());
```

结果：

```cpp
int
```

因为这里根本没有创建 `NonConstructible`。

---

# 5. `declval` 本质上是一个“编译期占位符”

可以把：

```cpp
std::declval<T>()
```

想象成：

```text
“给我一个 T 类型的假想表达式”
```

例如：

```cpp
std::declval<T>().foo()
```

就是：

```text
假想存在一个 T
      ↓
调用 foo()
      ↓
检查这个表达式
```

---

# 6. 它为什么经常和 `decltype` 一起出现？

因为：

```cpp
std::declval<T>()
```

负责：

> **构造一个假想表达式**

而：

```cpp
decltype(...)
```

负责：

> **获取这个表达式的类型**

所以：

```cpp
decltype(std::declval<T>().foo())
```

可以拆成：

```text
std::declval<T>()
        ↓
假想 T 对象

       .foo()
        ↓
构造调用表达式

decltype(...)
        ↓
获取表达式类型
```

两者是天然搭档。

---

# 7. `declval` 最常见的用途：获取成员函数返回值

例如：

```cpp
struct Foo
{
    int get();
};
```

模板：

```cpp
template <typename T>
using get_result_t =
    decltype(std::declval<T>().get());
```

然后：

```cpp
using R = get_result_t<Foo>;
```

得到：

```cpp
int
```

---

如果：

```cpp
struct Foo
{
    std::string get();
};
```

那么：

```cpp
get_result_t<Foo>
```

就是：

```cpp
std::string
```

---

# 8. 可以检查成员函数是否存在

例如：

```cpp
template <typename T>
using foo_result_t =
    decltype(std::declval<T>().foo());
```

如果：

```cpp
struct A
{
    void foo();
};
```

那么：

```cpp
foo_result_t<A>
```

合法。

但是：

```cpp
struct B
{
};
```

那么：

```cpp
foo_result_t<B>
```

无法实例化。

这就成为了早期 C++ **SFINAE** 技术的基础。

---

# 9. `declval` + SFINAE

经典代码：

```cpp
template <typename T>
auto test(int)
    -> decltype(
        std::declval<T>().foo(),
        std::true_type{}
    );

template <typename>
auto test(...)
    -> std::false_type;
```

然后：

```cpp
struct A
{
    void foo();
};

struct B
{
};
```

可以：

```cpp
static_assert(
    decltype(test<A>(0))::value
);
```

而：

```cpp
static_assert(
    !decltype(test<B>(0))::value
);
```

这里发生了：

```text
T = A
    ↓
std::declval<A>().foo()
    ↓
合法
    ↓
SFINAE 成功
```

而：

```text
T = B
    ↓
std::declval<B>().foo()
    ↓
非法表达式
    ↓
SFINAE
    ↓
候选被丢弃
```

所以 `declval` 经常出现在：

```cpp
decltype(...)
```

里面，然后参与 SFINAE。

---

# 10. `declval` 不需要构造对象

这是它最重要的价值。

例如：

```cpp
class Abstract
{
public:
    virtual void foo() = 0;
};
```

这是抽象类。

不能：

```cpp
Abstract obj;
```

但是可以：

```cpp
decltype(std::declval<Abstract>().foo())
```

完全没问题。

因为：

```cpp
std::declval<Abstract>()
```

只是用于编译期表达式推导。

---

# 11. `declval` 的声明长什么样？

标准库实现可以粗略理解成：

```cpp
template <typename T>
std::add_rvalue_reference_t<T> declval() noexcept;
```

你可以把它简化理解成：

```cpp
template <typename T>
T&& declval() noexcept;
```

但标准实际实现还需要处理特殊类型，因此不要把它简单等价成一个普通函数。

更重要的是：

> **它只有声明，没有真正的实现。**

而且：

> **它设计出来就是给 `decltype` 等未求值上下文使用的。**

---

# 12. 为什么没有实现也没关系？

例如：

```cpp
decltype(std::declval<Foo>().foo())
```

编译器只需要知道：

```cpp
std::declval<Foo>()
```

的类型是什么。

它并不需要真正执行：

```cpp
std::declval<Foo>()
```

因为 `decltype` 中的表达式属于：

> **unevaluated context（未求值上下文）**

也就是说：

```cpp
decltype(...)
```

里面的表达式不会真正运行。

---

# 13. `declval` 和 `sizeof` 也可以理解为类似思想

例如：

```cpp
sizeof(std::declval<T>().foo())
```

这里也不会真的调用：

```cpp
foo()
```

因为 `sizeof` 的操作数也是未求值上下文。

因此：

```cpp
std::declval
```

的核心使用环境就是：

```text
未求值上下文
```

尤其：

```cpp
decltype(...)
sizeof(...)
noexcept(...)
requires(...)
```

等场景。

---

# 14. `declval<T>()` 到底是什么值类别？

这个知识点对你后面学习完美转发很重要。

通常可以把：

```cpp
std::declval<T>()
```

理解为：

```cpp
T&&
```

因此它通常产生一个 **xvalue**。

例如：

```cpp
std::declval<Foo>()
```

可以近似理解为：

```cpp
Foo&&
```

所以：

```cpp
std::declval<T>().foo()
```

实际上是在模拟：

```cpp
T&&.foo()
```

这意味着它能够参与成员函数的 `&` / `&&` 限定选择。

---

# 15. `&` 限定成员函数

例如：

```cpp
struct Foo
{
    int get() &;
    double get() &&;
};
```

这里：

```cpp
get() &
```

只能被 lvalue 调用。

而：

```cpp
get() &&
```

只能被 rvalue 调用。

---

如果：

```cpp
decltype(std::declval<Foo&>().get())
```

那么：

```cpp
Foo&
```

是 lvalue。

所以调用：

```cpp
get() &
```

结果：

```cpp
int
```

---

而：

```cpp
decltype(std::declval<Foo>().get())
```

`declval<Foo>()` 可以看作 `Foo&&`。

所以选择：

```cpp
get() &&
```

结果：

```cpp
double
```

这个非常重要。

---

# 16. `declval<T&>()`

如果：

```cpp
struct Foo
{
    int get() &;
    double get() &&;
};
```

那么：

```cpp
decltype(std::declval<Foo&>().get())
```

得到：

```cpp
int
```

因为：

```text
declval<Foo&>()
      ↓
Foo&
      ↓
lvalue
      ↓
get() &
      ↓
int
```

---

# 17. `declval<T>()`

而：

```cpp
decltype(std::declval<Foo>().get())
```

得到：

```cpp
double
```

因为：

```text
declval<Foo>()
      ↓
Foo&&
      ↓
xvalue
      ↓
get() &&
      ↓
double
```

所以 `declval` 不只是：

> “避免构造对象”

还可以：

> **精确模拟不同 value category 下的表达式行为。**

---

# 18. `declval<const T>()`

同样可以模拟 const 对象：

```cpp
decltype(std::declval<const Foo>().get())
```

它可以帮助编译器判断：

```text
const Foo&&
```

上的 `get()` 是否存在以及返回什么。

---

# 19. `declval` + 成员变量

例如：

```cpp
struct Person
{
    int age;
};
```

可以：

```cpp
decltype(std::declval<Person>().age)
```

得到：

```cpp
int&&
```

为什么不是 `int`？

因为：

```cpp
std::declval<Person>()
```

是一个 xvalue。

访问：

```cpp
.age
```

得到的也是 xvalue。

所以：

```cpp
decltype(...)
```

按照普通表达式规则：

```text
xvalue → T&&
```

因此得到：

```cpp
int&&
```

---

# 20. 如果只想获取成员变量“声明类型”

可以使用：

```cpp
decltype(Person::age)
```

但注意这里语法和非静态成员并不一样。

更常见的做法是：

```cpp
using member_type =
    std::remove_cvref_t<
        decltype(std::declval<Person>().age)
    >;
```

得到：

```cpp
int
```

这也是为什么：

```text
declval
+
decltype
+
remove_cvref
```

经常一起出现。

---

# 21. `declval` + 运算符

它不仅能调用成员函数。

例如：

```cpp
template <typename T>
using add_result_t =
    decltype(
        std::declval<T>() +
        std::declval<T>()
    );
```

如果：

```cpp
struct Number
{
};
```

并定义：

```cpp
Number operator+(const Number&, const Number&);
```

那么：

```cpp
add_result_t<Number>
```

就可以得到：

```cpp
Number
```

这其实是在编译期询问：

> `T + T` 这个表达式是否合法？如果合法，结果是什么类型？

---

# 22. `declval` + 下标运算符

例如：

```cpp
template <typename T>
using subscript_result_t =
    decltype(
        std::declval<T>()[0]
    );
```

如果：

```cpp
std::vector<int>
```

那么：

```cpp
subscript_result_t<std::vector<int>>
```

得到：

```cpp
int&
```

因为：

```cpp
vector<int>::operator[]
```

通常返回：

```cpp
int&
```

---

# 23. `declval` + `operator*`

例如：

```cpp
template <typename T>
using deref_result_t =
    decltype(*std::declval<T>());
```

对于：

```cpp
int*
```

得到：

```cpp
int&
```

因为：

```cpp
*pointer
```

是 lvalue。

这在泛型库里非常常见。

---

# 24. `declval` + `noexcept`

例如：

```cpp
template <typename T>
constexpr bool foo_noexcept =
    noexcept(std::declval<T>().foo());
```

它可以在编译期检查：

> `T::foo()` 是否声明为 `noexcept`。

这也是现代 C++ 泛型代码非常常见的模式。

---

# 25. `declval` + `requires`

现代 C++20 中：

```cpp
template <typename T>
concept HasFoo =
    requires(T t)
    {
        t.foo();
    };
```

这种代码已经比传统：

```cpp
decltype(std::declval<T>().foo())
```

更加直观。

所以可以把两者理解成：

### C++11/14 时代

```cpp
decltype(std::declval<T>().foo())
```

### C++20

```cpp
requires(T t)
{
    t.foo();
}
```

它们解决的问题高度相关：

> **编译期检查某个表达式是否有效。**

---

# 26. `std::invoke_result` 已经封装了很多 `declval`

例如：

```cpp
std::invoke_result_t<F, Args...>
```

可以获取：

```cpp
std::invoke(f, args...)
```

的结果类型。

你可以把它理解成标准库帮你封装了类似：

```cpp
decltype(
    std::invoke(
        std::declval<F>(),
        std::declval<Args>()...
    )
)
```

所以学习 `declval` 能帮助你理解很多 `type_traits` 到底在干什么。

---

# 27. `declval` 与 `is_invocable`

例如：

```cpp
std::is_invocable_v<F, Args...>
```

实际上就是在问：

> `F` 能不能用 `Args...` 调用？

这类 trait 的底层思想就是：

```text
构造假想表达式
       ↓
检查表达式是否合法
       ↓
得到类型/true/false
```

而 `declval` 正是实现这类“假想表达式”的经典工具。

---

# 28. 一个完整的模板元编程例子

我们定义：

```cpp
struct A
{
    int foo();
};

struct B
{
    std::string foo();
};
```

然后：

```cpp
template <typename T>
using foo_result_t =
    decltype(std::declval<T>().foo());
```

于是：

```cpp
using AResult = foo_result_t<A>;
using BResult = foo_result_t<B>;
```

得到：

```text
AResult → int
BResult → std::string
```

进一步：

```cpp
static_assert(
    std::is_same_v<AResult, int>
);

static_assert(
    std::is_same_v<BResult, std::string>
);
```

这就是非常典型的：

```text
模板参数
   ↓
declval<T>()
   ↓
构造假想对象
   ↓
.foo()
   ↓
decltype
   ↓
得到返回类型
   ↓
type_traits
```

---

# 29. `declval` 最核心的三个用途

你可以直接记住：

## ① 不创建对象，模拟对象

```cpp
std::declval<T>()
```

---

## ② 获取表达式类型

```cpp
decltype(
    std::declval<T>().foo()
)
```

---

## ③ 检查表达式是否合法

```cpp
decltype(
    std::declval<T>().foo()
)
```

如果表达式非法，可以用于：

```text
SFINAE
```

或者现代：

```text
requires / concepts
```

---

# 30. `declval` 的脑内模型

以后看到：

```cpp
std::declval<T>()
```

你可以直接在脑子里替换成：

```cpp
T&&
```

例如：

```cpp
std::declval<T>().foo()
```

脑中可以先想成：

```cpp
T&&.foo()
```

然后问：

> 一个 `T&&` 对象能不能调用 `foo()`？

如果能：

```cpp
decltype(...)
```

再告诉你：

> 调用结果是什么类型？

---

# 31. 最重要的几个例子

```cpp
struct Foo
{
    int foo();
};
```

### 获取返回类型

```cpp
decltype(std::declval<Foo>().foo())
// int
```

### 获取左值对象调用结果

```cpp
decltype(std::declval<Foo&>().foo())
// int
```

### 获取 const 对象调用结果

```cpp
decltype(std::declval<const Foo>().foo())
// 如果 foo() 没有 const 限定，则非法
```

### 检查运算符

```cpp
decltype(
    std::declval<Foo>() +
    std::declval<Foo>()
)
```

### 检查下标

```cpp
decltype(
    std::declval<Foo>()[0]
)
```

### 检查解引用

```cpp
decltype(
    *std::declval<Foo>()
)
```

---

# 32. 最后记这一张关系图

你现在学习的几个知识点可以这样串：

```text
                    std::declval
                         │
                         ↓
                 构造假想表达式
                         │
                         ↓
                     decltype
                         │
                         ↓
                  获取表达式类型
                         │
              ┌──────────┴──────────┐
              ↓                     ↓
          type_traits              SFINAE
              │                     │
              ↓                     ↓
       is_same / remove...     表达式是否合法
              │                     │
              └──────────┬──────────┘
                         ↓
                   C++20 requires
                         ↓
                     Concepts
```

### 一句话记忆

> **`std::declval<T>()` = “给我一个假想的 `T` 对象，我只想拿它做编译期类型/表达式分析，不是真的创建它。”**

而你接下来学习模板元编程时，看到下面这种代码：

```cpp
decltype(std::declval<T>().foo())
```

应该立即翻译成：

> **“假装有一个 T，调用它的 foo()，看看这个调用是否合法以及返回什么类型。”**

这基本就是 `declval` 最核心的意义。