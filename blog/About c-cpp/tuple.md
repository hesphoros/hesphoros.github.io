# std::tuple
`std::tuple` 是 C++11 引入的一个**固定大小、可以存放不同类型元素的容器**。

如果你已经理解了 `std::pair`，那么可以把：

```cpp
std::pair<int, double>
```

理解成“只能放两个元素的 tuple”，而：

```cpp
std::tuple<int, double, std::string>
```

则可以放任意多个不同类型的元素。

---

# 1. 最基本的用法

```cpp
#include <tuple>
#include <string>

std::tuple<int, double, std::string> data{
    42,
    3.14,
    "hello"
};
```

它里面有三个元素：

```text
index 0 → int
index 1 → double
index 2 → string
```

和 `std::vector` 不一样：

```cpp
std::vector<int>
```

要求所有元素都是 `int`。

而 `tuple`：

```cpp
std::tuple<int, double, std::string>
```

每一个位置都可以有完全不同的类型。

---

# 2. 使用 `std::get` 访问

最基本的访问方式：

```cpp
auto x = std::get<0>(data);
auto y = std::get<1>(data);
auto z = std::get<2>(data);
```

得到：

```text
x → int
y → double
z → std::string
```

注意这里的：

```cpp
std::get<0>
```

`0` 是**编译期常量**。

也就是说：

```cpp
int i = 0;

std::get<i>(data); // ❌
```

不行。

必须：

```cpp
std::get<0>(data);
```

所以 `tuple` 本质上是一个**编译期索引的数据结构**。

---

# 3. 可以按照类型访问

如果类型唯一，还可以：

```cpp
std::get<int>(data);
std::get<double>(data);
std::get<std::string>(data);
```

例如：

```cpp
std::cout << std::get<int>(data);
```

输出：

```text
42
```

但是如果出现重复类型：

```cpp
std::tuple<int, int, double> t{1, 2, 3.14};
```

那么：

```cpp
std::get<int>(t); // ❌
```

因为有两个 `int`，编译器不知道你想要哪一个。

此时只能：

```cpp
std::get<0>(t);
std::get<1>(t);
```

---

# 4. Structured Binding

```cpp
std::tuple<int, double, std::string> data{
    42,
    3.14,
    "hello"
};

auto [a, b, c] = data;
```

现在：

```text
a → 42
b → 3.14
c → "hello"
```

所以：

```cpp
auto [a, b, c] = data;
```

本质上就是把 tuple 的元素拆开。

---

# 5. `auto&` 非常重要

例如：

```cpp
std::tuple<int, double> data{10, 3.14};

auto [a, b] = data;

a = 100;
```

此时：

```cpp
std::get<0>(data)
```

还是：

```text
10
```

因为 `a` 是一个拷贝。

如果：

```cpp
auto& [a, b] = data;

a = 100;
```

那么：

```cpp
std::get<0>(data)
```

就是：

```text
100
```

所以：

```cpp
auto [a, b]
```

和：

```cpp
auto& [a, b]
```

语义是不同的。

---

# 6. `std::make_tuple`

你不一定需要手写：

```cpp
std::tuple<int, double, std::string>
```

可以：

```cpp
auto t = std::make_tuple(
    42,
    3.14,
    "hello"
);
```

编译器自动推导类型。

大概得到：

```cpp
std::tuple<int, double, const char*>
```

然后：

```cpp
auto [a, b, c] = t;
```

---

# 7. `std::tie`

`tuple` 还有一个很经典的工具：

```cpp
std::tie
```

例如：

```cpp
int x;
double y;

std::tie(x, y) = std::make_tuple(42, 3.14);
```

执行以后：

```text
x = 42
y = 3.14
```

实际上 `std::tie` 创建的是：

```cpp
std::tuple<int&, double&>
```

也就是一个**引用组成的 tuple**。

这也是 C++17 Structured Binding 出现之前经常用于“接收多个返回值”的方式。

---

# 8. `tuple` 最重要的地方：编译期类型信息

假设：

```cpp
using T = std::tuple<int, double, std::string>;
```

你可以在编译期获取：

### tuple 有几个元素

```cpp
std::tuple_size_v<T>
```

结果：

```text
3
```

---

### 获取第 N 个元素的类型

```cpp
std::tuple_element_t<0, T>
```

得到：

```cpp
int
```

而：

```cpp
std::tuple_element_t<1, T>
```

得到：

```cpp
double
```

```cpp
std::tuple_element_t<2, T>
```

得到：

```cpp
std::string
```

于是：

```cpp
using A = std::tuple_element_t<0, T>;

static_assert(std::is_same_v<A, int>);
```

---

# 9.  Detection Idiom 

`std::tuple` 提供：

```cpp
std::tuple_size<T>
std::tuple_element<I, T>
std::get<I>(t)
```

这些都是**编译期操作**。

例如：

```cpp
template<typename T>
void print(T&& tuple)
{
    constexpr auto size = std::tuple_size_v<
        std::remove_reference_t<T>
    >;

    // ...
}
```

你可以在编译期知道：

```text
T 是什么 tuple
        ↓
tuple_size
        ↓
有几个元素
        ↓
tuple_element
        ↓
每个元素是什么类型
```



---

# 10. C++17 中遍历 tuple

这是模板元编程里非常经典的一段：

```cpp
template<typename Tuple, std::size_t... I>
void print_impl(
    const Tuple& t,
    std::index_sequence<I...>
)
{
    ((std::cout << std::get<I>(t) << '\n'), ...);
}

template<typename... Args>
void print(const std::tuple<Args...>& t)
{
    print_impl(
        t,
        std::index_sequence_for<Args...>{}
    );
}
```

然后：

```cpp
auto t = std::make_tuple(1, 3.14, "hello");

print(t);
```

就可以依次打印所有元素。

---

