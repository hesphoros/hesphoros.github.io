# Sturct Bunding

它的作用非常简单：

> **把一个对象里的多个值，一次性“拆出来”，分别绑定到变量。**

### 1. 最简单的例子

```cpp
std::pair<int, std::string> p{42, "hello"};

auto [number, text] = p;

std::cout << number; // 42
std::cout << text;   // hello
```

这里：

```cpp
auto [number, text] = p;
```

就是 Structured Binding。

相当于你以前需要写：

```cpp
auto number = p.first;
auto text = p.second;
```

现在可以直接：

```cpp
auto [number, text] = p;
```

---

### 2. `std::tuple`

它不仅支持 `pair`，还支持 `tuple`：

```cpp
std::tuple<int, double, std::string> data{
    10, 3.14, "hello"
};

auto [a, b, c] = data;

std::cout << a; // 10
std::cout << b; // 3.14
std::cout << c; // hello
```

这在 C++17 中非常常见。

---

### 3. 结构体也可以

```cpp
struct Person {
    std::string name;
    int age;
};

Person p{"Alice", 20};

auto [name, age] = p;

std::cout << name; // Alice
std::cout << age;  // 20
```

注意：

```cpp
auto [name, age] = p;
```

这里的 `name` 和 `age` **不是 `p.name` / `p.age` 的简单语法糖**，Structured Binding 背后有一套专门的语言规则。

---

### 4. `auto`、`auto&`、`const auto&`

这个非常重要。

#### `auto`

```cpp
Person p{"Alice", 20};

auto [name, age] = p;
```

相当于拆出一份值。

修改：

```cpp
name = "Bob";
age = 30;
```

不会修改 `p`。

---

#### `auto&`

```cpp
auto& [name, age] = p;
```

这时候绑定到原对象。

```cpp
name = "Bob";
age = 30;
```

那么：

```cpp
p.name // "Bob"
p.age  // 30
```

---

#### `const auto&`

```cpp
const auto& [name, age] = p;
```

以引用方式访问，但是不能修改：

```cpp
name = "Bob"; // ❌
```

这个形式在避免拷贝的时候很有用。

---

### 5. `std::map` 里特别常见

你以后写 C++ 基本会经常看到：

```cpp
std::map<std::string, int> map{
    {"Alice", 100},
    {"Bob", 200}
};

for (const auto& [name, score] : map) {
    std::cout << name << ": " << score << '\n';
}
```

以前可能写：

```cpp
for (const auto& item : map) {
    std::cout << item.first << ": "
              << item.second << '\n';
}
```

C++17：

```cpp
for (const auto& [name, score] : map)
```

可读性明显更好。

---

### 6. 函数返回多个值

这也是 Structured Binding 很重要的用途。

```cpp
std::pair<int, int> get_position()
{
    return {100, 200};
}

auto [x, y] = get_position();

std::cout << x << ", " << y;
```

可以把一个 `pair` 返回值直接拆成两个变量。

例如：

```cpp
auto [ptr, inserted] = map.insert({"hello", 123});
```

这里：

```text
ptr       → iterator
inserted  → bool
```

这在现代 C++ 中非常常见。

---

### 7. 和 `std::tie` 的区别

C++17 之前经常使用：

```cpp
int x;
double y;

std::tie(x, y) = get_value();
```

C++17 可以直接：

```cpp
auto [x, y] = get_value();
```

所以 Structured Binding 可以理解成：

> **C++17 给“把一个复合对象拆成多个变量”提供的原生语言支持。**

---