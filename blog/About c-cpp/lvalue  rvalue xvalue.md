在 C++ 中，值类别（Value Categories）是用来描述表达式（Expression）特性的分类标准。它决定了表达式是否具有持久的内存地址，以及是否可以被移动（Move）或拷贝（Copy）。

### 表达式分类体系

```
                   值类别 (Value Category)
                                / \
                               /   \
                              /     \
                          glvalue   rvalue
                           /   \   /   \
                          /     \ /     \
                     lvalue    xvalue    prvalue

```

---

### 核心值类别对比

| 分类                 | 核心特征                                      | 是否有标识（Identity）？ | 是否可被移动（Can be Moved）？ | 典型示例                                           |
| -------------------- | --------------------------------------------- | ------------------------ | ------------------------------ | -------------------------------------------------- |
| **lvalue** (左值)    | 指向持久的内存位置（有变量名/可取地址）       | **是**                   | **否**（默认进行拷贝）         | `a`、`*ptr`、`"hello"`（字符串字面量）、`arr[2]`   |
| **prvalue** (纯右值) | 临时值或计算结果，用于计算或初始化对象        | **否**                   | **是**                         | `42`、`a + b`、`std::string("hi")`、非引用返回函数 |
| **xvalue** (将亡值)  | 拥有标识但生命周期即将结束，资源可被回收/移动 | **竞/是**                | **是**                         | `std::move(x)`、`static_cast<T&&>(x)`              |

---

### 组合分类定义

* **glvalue** (泛左值，Generalized Lvalue)：包含 **lvalue** 和 **xvalue**。**拥有标识**的所有表达式统称。
* **rvalue** (右值)：包含 **prvalue** 和 **xvalue**。**资源可以被移动/转移**的所有表达式统称。

---

### 详细解析与代码示例

#### 1. lvalue (左值)

具有明确内存地址且生命周期跨越当前语句的表达式。可以通过取地址符 `&` 获取其内存地址。

```cpp
int x = 10;          // 'x' 是 lvalue
int* p = &x;         // 正确：x 有具体的内存地址
int& ref = x;        // 左值引用绑定到 lvalue

std::string s1 = "hello"; // 's1' 是 lvalue，"hello" 是字符串字面量（也是 lvalue）

```

#### 2. prvalue (纯右值)

没有独立内存地址的纯粹临时值。C++17 起，prvalue 主要作为“初始化指令”（支持强制拷贝消除/Copy Elision）。

```cpp
int x = 5 + 3;       // '5 + 3' 是 prvalue
std::string get_str() { return "temp"; }

std::string s2 = get_str(); // 'get_str()' 的返回值是 prvalue

```

#### 3. xvalue (将亡值)

既有标识又即将销毁的对象（eXpiring value）。通常是显式转换为右值引用的对象，以便把资源“移走”而不是复制。

```cpp
std::string a = "Hello World";

// std::move(a) 的结果是一个 xvalue
// 它将 'a' 从 lvalue 强转为右值引用类型 (std::string&&)
std::string b = std::move(a); 

```

---

### 引用绑定规则

不同类型的值类别在重载决议中绑定到不同的引用类型：

```cpp
void process(int& lref)      { /* 仅匹配 lvalue */ }
void process(int&& rref)     { /* 匹配 rvalue (prvalue 和 xvalue) */ }
void process(const int& cref){ /* 可匹配任意值类别 */ }

int x = 10;

process(x);            // 调用 process(int&)
process(10);           // 调用 process(int&&) - prvalue
process(std::move(x)); // 调用 process(int&&) - xvalue

```
