# 1. ADL 是什么？

ADL 全称：

> **Argument-Dependent Lookup**
>
> 参数依赖查找

它是 C++ 的一种**函数查找规则**。

最简单的例子：

```cpp
namespace Foo {
    struct X {};

    void hello(X);
}

int main() {
    Foo::X x;

    hello(x);   // 可以找到 Foo::hello
}
```

你可能会疑惑：

```cpp
hello(x);
```

明明没有：

```cpp
Foo::hello(x);
```

为什么还能找到？

因为参数：

```cpp
x
```

的类型是：

```cpp
Foo::X
```

所以 C++ 会根据参数类型，额外去 `Foo` 这个关联命名空间里面寻找 `hello`。

这就是 **ADL**。

---

# 2. 普通查找 vs ADL

例如：

```cpp
namespace Foo {
    struct X {};
    void hello(X);
}
```

调用：

```cpp
hello(Foo::X{});
```

C++ 大致会做两件事：

```text
hello(...)
   │
   ├── 普通名字查找
   │
   └── ADL 参数依赖查找
             │
             ▼
          Foo::hello
```

所以：

```cpp
hello(Foo::X{});
```

能够找到：

```cpp
Foo::hello(Foo::X)
```

---

