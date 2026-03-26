核心：**Copy-On-Write（写时拷贝，COW）=“读共享，写时复制”**。

# 什么是 Copy-On-Write？



> 多个对象先**共享同一份数据**，只有当某个对象**要修改数据时**，才**拷贝一份新的再改**。

---

## 示例

```cpp
std::vector<int> a = {1,2,3};
auto b = a;  // 这里如果是 COW，不会真的复制
```

在 COW 里：

* a 和 b **共享同一块内存**
* 没有发生拷贝（节省性能）

直到：

```cpp
b.push_back(4);
```

这时：

1. 检测到 **共享数据**
2. 给 b **拷贝一份新的 vector**
3. 在新副本上修改

---

# 底层实现逻辑

核心机制就两个：

---

## 1. 引用计数（Reference Counting）

```cpp
struct Buffer {
    int*     data;
    size_t   size;
    int ref_count;
};
```

多个对象共享：

```cpp
a.buffer = buffer;
b.buffer = buffer;
buffer->ref_count = 2;
```

---

## 2. 写时检查（核心）

每次“可能修改”的操作：

```cpp
void ensure_unique() {
    if (buffer->ref_count > 1) {
        // 拷贝一份
        Buffer* newBuf = copy(buffer);
        buffer->ref_count--;
        buffer = newBuf;
    }
}
```

然后再修改：

```cpp
ensure_unique();
buffer->data[i] = 42;
```

---

##  流程总结

```
多个对象共享 → ref_count > 1
        ↓
发生写操作
        ↓
检测共享
        ↓
复制数据
        ↓
只修改自己的副本
```

---

#  现代 C++ 的现实情况

###  `std::vector` 不使用 COW

原因：

1. **线程安全问题（data race）**
2. **迭代器语义会崩**
3. C++11 后标准明确倾向禁止

历史上：

* `std::string` 在老实现里用过 COW
* 现在（C++11 之后）基本都取消了

---

# 遍历 vector 时修改

---

## 方案1：延迟修改（推荐）

```cpp
std::vector<int> to_add;
std::vector<int> to_remove;
```

遍历时只记录：

```cpp
for (auto& x : vec) {
    if (...) to_add.push_back(...);
}
```

遍历后再处理：

```cpp
// remove / add
```

 优点：

* ✅ 安全
* ✅ 不破坏迭代器
* ✅ STL 标准推荐方式

---

##  方案2：Copy-On-Write

理论上：

```cpp
auto vec2 = vec; // 共享
for (...) {
    vec2.push_back(...); // 触发复制
}
```

---

### 实际问题

在 `std::vector` 上：

* 根本没有 COW
*  修改会直接影响容器结构
*  iterator 直接失效 

