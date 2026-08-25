# C++ 模板特化（Template Specialization）

模板描述一组通用实现，特化则为某些模板实参提供更合适的实现。

例如，通用序列化器可以使用流输出大多数类型，但 `bool` 更适合输出 `true` 或 `false`。这时不必在主模板中堆叠类型判断，可以为 `bool` 单独提供一个特化版本。

模板特化主要分为两类：

| 类型 | 含义 | 示例 |
| --- | --- | --- |
| 全特化（full specialization） | 模板实参全部确定 | `Serializer<bool>` |
| 偏特化（partial specialization） | 只限定一部分实参或一种类型形式 | `TypeName<T*>` |

> 类模板和变量模板支持全特化与偏特化；函数模板只支持全特化，不支持偏特化。函数模板通常使用重载代替偏特化。

## 1. 主模板

主模板（primary template）是默认实现，特化版本必须建立在它的声明之后。

```cpp
#include <iostream>

template <typename T>
struct Serializer {
    static void write(const T& value) {
        std::cout << value;
    }
};
```

没有特化匹配时，编译器会实例化主模板：

```cpp
Serializer<int>::write(42);  // 使用 Serializer<T>
```

## 2. 类模板全特化

全特化为一组完全确定的模板实参提供实现。语法特征是空模板参数列表 `template <>`：

```cpp
template <>
struct Serializer<bool> {
    static void write(bool value) {
        std::cout << (value ? "true" : "false");
    }
};

Serializer<bool>::write(true);  // 使用 Serializer<bool>
```

可以这样理解：

```cpp
template <typename T>  // 主模板：T 仍然未知
struct Serializer;

template <>            // 全特化：模板参数已经全部确定
struct Serializer<bool>;
```

全特化是一个独立定义，不会自动继承主模板的成员。特化版本需要自行提供调用方依赖的接口。

```cpp
template <typename T>
struct Box {
    void open();
    void close();
};

template <>
struct Box<int> {
    void open();
    // Box<int> 不会自动拥有 close()
};
```

## 3. 类模板偏特化

偏特化保留部分模板参数，用来匹配一类类型，而不是某一个确定类型。

### 3.1 按类型形式特化

下面的偏特化匹配任意指针类型：

```cpp
template <typename T>
struct TypeName {
    static constexpr const char* value = "value";
};

template <typename T>
struct TypeName<T*> {
    static constexpr const char* value = "pointer";
};

TypeName<int>::value;     // "value"，使用主模板
TypeName<int*>::value;    // "pointer"，使用偏特化
TypeName<double*>::value; // "pointer"，使用偏特化
```

偏特化的 `template <typename T>` 不为空，因为匹配成功后仍需推导 `T`。

### 3.2 按部分模板参数特化

```cpp
template <typename T, typename U>
struct PairTraits {
    static constexpr bool same = false;
};

template <typename T>
struct PairTraits<T, T> {
    static constexpr bool same = true;
};

static_assert(!PairTraits<int, double>::same);
static_assert(PairTraits<int, int>::same);
```

`PairTraits<T, T>` 不是全特化，因为 `T` 仍然未知；它限定的是“两个模板实参类型相同”这一类情况。

### 3.3 按非类型模板参数特化

```cpp
#include <cstddef>

template <typename T, std::size_t N>
struct Storage {
    T data[N];
};

template <typename T>
struct Storage<T, 0> {
    // 对长度为 0 的情况提供独立实现
};
```

这里固定了 `N == 0`，但仍保留类型参数 `T`，所以它也是偏特化。

## 4. 多个特化如何匹配

当多个版本都可能匹配时，编译器选择约束更具体的版本：

```cpp
template <typename T>
struct Category {
    static constexpr const char* value = "primary";
};

template <typename T>
struct Category<T*> {
    static constexpr const char* value = "pointer";
};

template <typename T>
struct Category<const T*> {
    static constexpr const char* value = "pointer to const";
};

template <>
struct Category<const int*> {
    static constexpr const char* value = "pointer to const int";
};
```

对应关系如下：

| 实例 | 选择的版本 |
| --- | --- |
| `Category<double>` | 主模板 `Category<T>` |
| `Category<double*>` | 偏特化 `Category<T*>` |
| `Category<const double*>` | 更具体的偏特化 `Category<const T*>` |
| `Category<const int*>` | 全特化 `Category<const int*>` |

如果两个偏特化都能匹配，但编译器无法判断哪个更具体，程序会因特化匹配歧义而无法编译。

## 5. 函数模板特化

### 5.1 函数模板可以全特化

```cpp
#include <iostream>

template <typename T>
void print(const T& value) {
    std::cout << value;
}

template <>
void print<bool>(const bool& value) {
    std::cout << (value ? "true" : "false");
}
```

### 5.2 函数模板不能偏特化

下面的写法不合法：

```cpp
template <typename T>
void print(T value);

// 错误：函数模板不能偏特化
template <typename T>
void print<T*>(T* value);
```

应当使用函数重载：

```cpp
template <typename T>
void print(T value) {
    std::cout << value;
}

template <typename T>
void print(T* value) {
    std::cout << "pointer: " << value;
}
```

实践中，即使函数模板可以全特化，也通常优先使用重载。函数调用会先进行重载决议，再处理所选函数模板的特化；将两套规则混在一起容易产生不直观的结果。

## 6. 成员函数全特化

不必特化整个类模板，也可以只特化其中一个成员函数：

```cpp
#include <iostream>

template <typename T>
struct Printer {
    void print() const {
        std::cout << "generic";
    }
};

template <>
void Printer<int>::print() const {
    std::cout << "int";
}
```

这只改变 `Printer<int>::print()`；类模板的其他成员仍来自主模板。

## 7. 变量模板特化（C++14）

变量模板同样支持全特化和偏特化：

```cpp
template <typename T>
constexpr bool is_pointer_v = false;

template <typename T>
constexpr bool is_pointer_v<T*> = true;

static_assert(!is_pointer_v<int>);
static_assert(is_pointer_v<int*>);
```

标准库中的许多类型萃取（type traits）都采用类似结构：主模板给出默认结论，偏特化识别特定类型形式。

## 8. 声明与可见性规则

显式特化必须满足几个关键条件：

1. 先声明主模板，再声明特化。
2. 特化必须在第一次导致隐式实例化之前可见。
3. 显式特化应声明在主模板所属的命名空间中。
4. 放在头文件中的非模板函数或静态数据成员定义仍要遵守 ODR；需要时使用 `inline`。

```cpp
template <typename T>
struct Handler;

template <>
struct Handler<int>;  // 可以先声明特化

// Handler<int> 的完整定义可以放在后面
template <>
struct Handler<int> {
    void run();
};
```

如果某个翻译单元已经根据主模板实例化了 `Handler<int>`，随后才看到 `Handler<int>` 的显式特化，程序通常会报“实例化后特化”之类的错误。

## 9. 特化、重载与 `if constexpr` 的选择

| 需求 | 更合适的工具 |
| --- | --- |
| 为类模板的某个确定类型提供完整实现 | 全特化 |
| 匹配指针、数组、函数签名等一类类型形式 | 偏特化 |
| 改变函数对不同参数的行为 | 函数重载 |
| 实现类型萃取或编译期类型分类 | 类模板或变量模板偏特化 |
| 实现主体大致相同，仅少量分支不同 | `if constexpr` |
| 对模板参数表达语义约束（C++20） | concepts / `requires` |

特化适合“实现结构本身不同”的情况。如果只是函数体中的一个小分支，`if constexpr` 往往更直接：

```cpp
#include <type_traits>

template <typename T>
void process(T value) {
    if constexpr (std::is_pointer_v<T>) {
        // 处理指针
    } else {
        // 处理普通值
    }
}
```

## 10. 常见错误

### 10.1 混淆全特化和偏特化

```cpp
template <typename T>
struct Wrapper;          // 主模板

template <>
struct Wrapper<int>;     // 全特化：没有待推导参数

template <typename T>
struct Wrapper<T*>;      // 偏特化：仍需推导 T
```

判断方法很简单：如果特化后的模板实参中仍有参数需要推导，就是偏特化。

### 10.2 认为特化会继承主模板接口

全特化是独立实现。主模板新增成员后，已有全特化不会自动获得该成员，因此公共接口变化时要同步检查所有全特化。

### 10.3 用函数模板特化模拟重载

函数模板全特化不是普通重载，参与选择的顺序也不同。需要根据参数类型选择函数时，优先编写重载；需要根据类型生成不同类结构时，再使用类模板特化。

## 总结

模板特化的核心是：在保留通用模板的同时，为更具体的模板实参提供实现。

```text
主模板       template <typename T> struct X;
全特化       template <> struct X<int>;
偏特化       template <typename T> struct X<T*>;
成员全特化   template <> void X<int>::func();
```

记住两条最实用的规则：

- 类模板可以全特化和偏特化。
- 函数模板不能偏特化，需要这种效果时使用函数重载。
