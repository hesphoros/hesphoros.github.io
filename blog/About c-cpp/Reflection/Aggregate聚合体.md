**Aggregate（聚合体）**，简单说就是：

> **把多个相关的东西组合成一个整体，并把这个整体作为一个单独的对象来使用。**

这个词在 C++ 里尤其常见，因为 **Aggregate Type（聚合类型）** 是 C++ 语言标准里的一个正式概念。

### 1. 最直观的理解

比如：

```cpp
struct Person
{
    std::string name;
    int age;
};
```

这里 `Person` 可以看成一个聚合体：

```text
Person
├── name
└── age
```

它把 `name` 和 `age` **聚合**成了一个 `Person`。

于是你可以：

```cpp
Person p{"Alice", 18};
```

这就是**聚合初始化（aggregate initialization）**。

---

### 2. C++ 中的 Aggregate 是什么

在 C++ 语境下，Aggregate 不是简单地说“成员很多”。

它特指一种满足语言规则的类型。

典型例子：

```cpp
struct Point
{
    int x;
    int y;
};
```

可以：

```cpp
Point p{10, 20};
```

而不需要自己写构造函数：

```cpp
struct Point
{
    int x;
    int y;

    // 没有构造函数
};
```

这就是典型的 Aggregate。

---

### 3. 为什么 C++ 反射里经常看到 Aggregate？

这个对你现在研究的**编译期反射**特别重要。

因为很多 C++ 反射库会利用：

```cpp
T{...}
```

这种 **Aggregate Initialization** 的特性，来“探测”一个类型到底有多少个成员。

例如：

```cpp
struct User
{
    int id;
    std::string name;
    bool enabled;
};
```

反射库可能尝试：

```cpp
User{arg1}
User{arg1, arg2}
User{arg1, arg2, arg3}
User{arg1, arg2, arg3, arg4}
```

如果：

```cpp
User{a, b, c}
```

能够编译，而：

```cpp
User{a, b, c, d}
```

不能编译，

那么它就可以推断：

```text
User 大约有 3 个可聚合初始化成员
```

这就是很多**无侵入式 C++ 编译期反射**技术的基础之一。

---

### 4. Aggregate ≠ 普通的“集合”

这两个概念要区分：

```text
Aggregate
    ↓
语言层面的“聚合类型”

Collection / Container
    ↓
用于存储多个元素的容器
```

例如：

```cpp
std::vector<int>
```

是一个容器，但通常不会叫它 C++ 的 Aggregate。

而：

```cpp
struct Point
{
    int x;
    int y;
};
```

是 Aggregate Type。

---

它的核心思路之一就是：

```text
普通 struct
     ↓
Aggregate
     ↓
Aggregate initialization
     ↓
推测成员数量
     ↓
tuple-like 访问成员
     ↓
编译期反射
```

例如：

```cpp
struct Person
{
    int age;
    double height;
};
```

可以做到类似：

```cpp
boost::pfr::get<0>(person); // age
boost::pfr::get<1>(person); // height
```

甚至：

```cpp
boost::pfr::for_each_field(person, [](auto& field) {
    // ...
});
```

**但它最大的限制也正是 Aggregate 本身：**

它主要能处理“长得像普通 struct”的类型，而不能天然获得：

```cpp
成员名字
成员访问权限
成员函数
继承关系
属性/注解
```

这些信息。

---


> **Aggregate Reflection / Aggregate-based Reflection**

你可以直接理解成：

> **利用 C++ 的聚合初始化机制，在编译期“猜出”一个 struct 的成员结构，从而实现反射。**

这也是为什么你之前想自己做一个**高性能、编译期、零运行时开销的反射库**时，**Aggregate 是非常值得深入研究的一个核心概念。**

# Aggregate Class

满足以下条件的类才可以称之为聚合类
1. 无自定义构造函数；
2. 非静态数据成员没有大括号或等号初始化器，即类内没有初始值；
3. 无私有或保护的非静态成员；
4. 无基类和虚函数。