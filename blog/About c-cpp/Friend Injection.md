# Friend Injection 是什么？

来看：

```cpp
template<typename T>
struct X {
    friend void foo() {}
};
```

这里：

```cpp
friend void foo() {}
```

并不是：

```cpp
X::foo()
```

它是一个**非成员函数**。

也就是说，它不是：

```cpp
struct X {
    void foo(); // 成员函数
};
```

而更接近：

```cpp
struct X {
    friend void foo() {}
};
```

这里的 `foo()` 是一个**外部函数**。

---

# 为什么叫 Friend？

因为：

```cpp
friend
```

原本的主要作用是：

> 允许某个外部函数访问类的 `private/protected` 成员。

例如：

```cpp
class A {
private:
    int value = 123;

    friend void print(A const& a);
};

void print(A const& a) {
    std::cout << a.value;
}
```

这里：

```cpp
print()
```

是类 `A` 的 friend。

但有一个很有意思的特性：

```cpp
friend void foo() {}
```

这种定义可以**直接在类定义里面定义一个非成员函数**。

---

# 结合摸板

比如：

```cpp
template<typename T>
struct X {
    friend void foo(X) {}
};
```

实例化：

```cpp
X<int> x;
```

实际上会产生类似：

```cpp
void foo(X<int>);
```

如果：

```cpp
X<double> y;
```

又会产生：

```cpp
void foo(X<double>);
```

于是：

```text
X<int>     → foo(X<int>)
X<double>  → foo(X<double>)
X<string>  → foo(X<string>)
```

这就是 **Friend Injection** 的基本思想。



---

这是理解这个技巧的关键

---

# 一个非常经典的例子

C++ 标准库里面其实就有类似思想。

例如：

```cpp
std::swap(a, b);
```

通常推荐：

```cpp
using std::swap;
swap(a, b);
```

为什么？

因为：

```cpp
swap(a, b);
```

既可以找到：

```cpp
std::swap
```

又可以通过 ADL 找到类型自己定义的：

```cpp
swap(MyType&, MyType&)
```

也就是：

```text
swap(a, b)
   │
   ├── 普通查找 → std::swap
   │
   └── ADL → MyNamespace::swap
```

这是 ADL 最实际的用途之一。

---

# Friend Injection 和普通 Friend 的区别

需要稍微区分一下。

普通 friend：

```cpp
class A {
    friend void foo(A);
};
```

主要是：

> **声明某个函数可以访问 `A` 的私有成员。**

而 Friend Injection 技巧关注的是：

```cpp
template<typename T>
struct A {
    friend void foo(A) {}
};
```

利用模板实例化，让不同的：

```cpp
A<T>
```

产生不同的 friend 函数。

于是可以利用：

```text
模板参数
    ↓
friend 函数签名 / 返回类型
    ↓
ADL
    ↓
decltype
    ↓
把类型信息取回来
```




