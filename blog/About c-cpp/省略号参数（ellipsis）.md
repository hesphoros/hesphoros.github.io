# ellipsis`...`

在 C 语言中，**省略号参数（ellipsis）**就是函数参数列表里的 `...`：

```c
void func(int count, ...);
```

它表示：

> **函数除了已经声明的参数之外，还可以接受数量和类型都不固定的额外参数。**

这就是 C 语言实现**可变参数函数（variadic function）**的基础。

---

## 最经典的例子：`printf`

你平时写：

```c
printf("%d %s %.2f\n", 123, "hello", 3.14);
```

实际上 `printf` 的声明大致是：

```c
int printf(const char *format, ...);
```

这里：

```c
const char *format
```

是一个普通参数。

而：

```c
...
```

表示后面还可以有任意数量的参数。

所以这些调用都是合法的：

```c
printf("hello\n");

printf("%d\n", 123);

printf("%d %d\n", 123, 456);

printf("%s %d %.2f\n", "hello", 123, 3.14);
```

---

# `...` 到底意味着什么?

例如：

```c
void test(int a, ...);
```

可以理解成：

```text
test(
    一个 int 参数,
    后面还有一些参数
)
```

比如：

```c
test(1);
test(1, 2);
test(1, 2, 3);
test(1, 2, 3, 4);
```

这些从**语法上**都可以。

但是有一个非常重要的问题：

> `...` 本身并没有告诉编译器后面的参数是什么类型、多少个。

例如：

```c
void test(int a, ...);

test(10, 20, 30, 40);
```

编译器知道：

```text
第一个参数：int
后面的：???
```

它不知道后面到底是：

```text
int
int
int
```

还是：

```text
double
char *
int
```

所以 C 语言需要另外一种机制来读取这些参数。

---

# `<stdarg.h>`：读取省略号参数

C 标准库提供了：

```c
#include <stdarg.h>
```

里面最重要的东西有：

```c
va_list
va_start
va_arg
va_end
```

例如：

```c
#include <stdarg.h>
#include <stdio.h>

void test(int count, ...)
{
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; ++i) {
        int value = va_arg(args, int);
        printf("%d\n", value);
    }
    va_end(args);
}

int main()
{
    test(3, 10, 20, 30);
}
```

输出：

```text
10
20
30
```

---

# 这几个东西分别干什么？

这是理解 C 可变参数的核心。

### `va_list`

```c
va_list args;
```

可以把它理解成：

> **一个用于遍历省略号参数的状态对象。**

---

### `va_start`

```c
va_start(args, count);
```

告诉程序：

> 从 `count` 后面的第一个省略号参数开始读取。

也就是：

```c
test(3, 10, 20, 30);
     ↑
   count

        ↑
        从这里开始
```

---

### `va_arg`

```c
va_arg(args, int)
```

表示：

> 取出下一个参数，并且把它当作 `int`。

例如：

```c
int a = va_arg(args, int);
int b = va_arg(args, int);
int c = va_arg(args, int);
```

那么：

```c
test(3, 10, 20, 30);
```

就会得到：

```text
a = 10
b = 20
c = 30
```

---

### `va_end`

```c
va_end(args);
```

结束参数遍历。

完整结构通常就是：

```c
void func(int count, ...)
{
    va_list args;

    va_start(args, count);

    // va_arg(...)

    va_end(args);
}
```

可以把它记成：

```text
va_start
   ↓
va_arg
   ↓
va_arg
   ↓
va_arg
   ↓
va_end
```

---

# 一个非常重要的问题：C 怎么知道参数类型？

答案是：

> **C 语言的 `...` 不会自动记录类型。**

例如：

```c
void test(int count, ...)
{
    va_list args;
    va_start(args, count);

    int x = va_arg(args, int);

    va_end(args);
}
```

调用：

```c
test(1, 123);
```

正确。

但是：

```c
test(1, 3.14);
```

你却使用：

```c
va_arg(args, int);
```

去读取。

这就产生了**未定义行为**。

所以可变参数函数一般必须通过某种方式让调用者和函数本身约定参数类型。

---

# `printf` 是怎么解决这个问题的？

`printf` 有：

```c
printf("%d %f %s", 123, 3.14, "hello");
```

这里：

```c
"%d %f %s"
```

就是**类型信息**。

它告诉 `printf`：

```text
%d → 后面读取 int

%f → 后面读取 double

%s → 后面读取 char*
```

所以可以把：

```c
printf("%d %s", 123, "hello");
```

理解成：

```text
format：
    第一个额外参数 → int
    第二个额外参数 → char*
```

然后 `printf` 内部通过类似：

```c
va_arg(args, int);
va_arg(args, char *);
```

把它们取出来。

---

# 它和普通函数参数有什么区别？

普通函数：

```c
void func(int a, double b, const char *c);
```

编译器明确知道：

```text
参数 1 → int
参数 2 → double
参数 3 → char*
```

而：

```c
void func(int a, ...);
```

编译器只知道：

```text
参数 1 → int
参数 2+ → 不知道
```

所以：

```c
func(10, 20, 30, 40);
```

编译器不会因为 `20, 30, 40` 数量不同而报错。

---

# 默认参数提升

使用 `...` 时，还涉及 C 语言的**默认参数提升（default argument promotions）**。

例如：

```c
float f = 3.14f;
char  c = 'A';
short s = 10;

func(3, f, c, s);
```

进入 `...` 后，这些类型会发生提升：

```text
float  → double
char   → int
short  → int
```

因此你读取时不能简单地写：

```c
va_arg(args, float);   // 错误思路
```

而应该：

```c
va_arg(args, double);
```

同样：

```c
char
short
```

通常需要按：

```c
int
```

读取。

---

# `...` 不等于 C++ 的模板参数包

这个特别容易混淆。

```c
void func(int count, ...);
```

是：

> **运行时可变参数**

而 C++：

```cpp
template <typename... Args>
void func(Args... args);
```

是：

> **编译期参数包（parameter pack）**

例如：

```cpp
func(1, 2.0, "hello");
```

C++ 编译器实际上知道：

```text
Args = {
    int,
    double,
    const char*
}
```

而 C 的：

```c
void func(...);
```

编译器并不会建立这样的类型列表。

所以可以简单记：

```text
C 的 ...
    ↓
运行时可变参数
    ↓
va_list / va_start / va_arg / va_end


C++ 的 template<typename... Args>
    ↓
编译期参数包
    ↓
类型系统知道每个参数的类型
```

---
