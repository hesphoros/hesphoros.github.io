`mutable` 关键字是 C++ 中的一个特殊修饰符，主要用于**修饰类的非静态成员变量**

允许在 `const` 成员函数中修改被 `mutable` 修饰的成员变量的值。

---

### 作用与场景

#### 1. 突破 `const` 限制

在 C++ 中，如果一个成员函数被声明为 `const`，它承诺不会改变对象的状态（即不会修改任何非 `mutable` 的成员变量）。

然而，有时我们需要在 `const` 成员函数中修改一些不影响对象**逻辑状态**（Logical State）的成员变量，比如用于缓存、统计或同步的辅助数据。这时就需要使用 `mutable`。

**示例：缓存/延迟计算 (Lazy Evaluation)**

假设你有一个 `compute()` 函数非常耗时，你想在第一次调用时计算结果并缓存起来，以便后续调用直接返回缓存值。

C++

```cpp
class DataProcessor {
public:
    int getValue() const {
        if (!m_cached) {
            // 这是一个 const 函数，但我们需要修改 m_result 和 m_cached
            // 如果没有 mutable，这里会导致编译错误
            m_result = heavyComputation(); 
            m_cached = true;
        }
        return m_result;
    }

private:
    int heavyComputation() const;

    // 逻辑状态：用户关心的数据结果
    int m_data; 
    
    // 辅助状态：用户不关心的内部缓存和标记
    mutable int m_result = 0;   // 允许在 const 函数中修改
    mutable bool m_cached = false; // 允许在 const 函数中修改
};
```

#### 2. 结合互斥锁 (Mutex)

在并发编程中，如果一个类需要使用互斥锁来保护其内部数据，但其**公共接口**要求是 `const` 函数（因为它不改变用户的逻辑数据），这时互斥锁变量就必须是 `mutable` 的。

C++

```cpp
#include <mutex>

class ThreadSafeData {
public:
    int getData() const {
        // 允许在 const 函数中锁定互斥锁
        std::lock_guard<std::mutex> lock(m_mutex); 
        return m_data;
    }
private:
    int m_data;
    // 互斥锁是对象的“物理”状态，但不是“逻辑”状态，需要 mutable 修饰
    mutable std::mutex m_mutex; 
};
```

---

###  总结

|**关键字**|**作用范围**|**含义**|
|---|---|---|
|**`const`** (修饰成员函数)|整个对象|**承诺**该函数不会修改任何非 `mutable` 的成员变量。|
|**`mutable`** (修饰成员变量)|特定成员变量|**打破承诺**，允许这个成员变量在 `const` 成员函数中被修改。|
