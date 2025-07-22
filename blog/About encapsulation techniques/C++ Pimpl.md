# Pointer to IMPLementation
🧠 _pimpl idiom_ 是一种新式 C++ 技术，用于隐藏实现、最小化耦合和分离接口。

Pimpl 是“指向实现的指针”的缩写。你可能已经熟悉这个概念，但知道它的其他名称，如柴郡猫或编译器防火墙成语。
# 基本原理
PIMPL的核心思想是将类的实现细节（如成员变量、私有方法等）从头文件中移到实现文件（cpp）中，通过一个指向实现细节的指针（通常用`std::unique_ptr`或裸指针）进行访问。

## 为何使用 pimpl？

下面是 pimpl idiom 如何改善软件开发生命周期：

- **隐藏实现细节**  
    用户只看到接口，类的实现细节不暴露在头文件中。
    
- **减少编译依赖**  
    修改实现类（Impl）不会引起头文件变化，从而不会触发大量文件重编译。
    
- **提高 ABI 稳定性**  
    改变实现类不影响类的大小和布局，动态链接时更安全。
    
## Pimpl 标头
🧱 基本实现

```cpp
// 头文件只暴露接口
#pragma once
#include <memory>

class MyClass {
public:
    MyClass();
    ~MyClass(); // 需要自定义析构函数
    void doSomething();

private:
    struct Impl;           // 前向声明
    std::unique_ptr<Impl> pImpl; // 指向实现的指针
};
```

pimpl idiom 避免重新生成级联和脆对象布局。 它非常适合（可传递）常用类型。

## Pimpl 实现

在 .cpp 文件中定义 `impl` 类。


```cpp
// my_class.cpp
#include "MyClass.h"
#include <iostream>

// 实现结构体的具体内容
struct MyClass::Impl {
    int secretData;
    void helper() {
        std::cout << "Helper function, secretData=" << secretData << std::endl;
    }
};

MyClass::MyClass() : pImpl(std::make_unique<Impl>()) {
    pImpl->secretData = 42;
}

MyClass::~MyClass() = default; // unique_ptr自动回收

void MyClass::doSomething() {
    pImpl->helper();
}
```


# PIMPL 的优点
1. **减少编译依赖**  
    头文件只暴露指针和接口，成员变量和私有实现完全隐藏。头文件变动少，减少了编译时的重新依赖。
    
2. **隐藏实现细节**  
    用户只看到接口，具体实现对外不可见，符合“最小可见性原则”。
    
3. **ABI稳定**  
    类的大小和布局在头文件里固定，方便二进制兼容（DLL/so升级时接口不变，用户代码无需重编译）。
    
4. **加快编译速度**  
    由于头文件变动极小，减少了大规模头文件包含导致的编译浪费。

# PIMPL的缺点
- **增加运行时开销**：多了一次指针间接访问（通常可忽略，但在极度性能敏感场合需注意）。
- **代码稍复杂**：需手动管理Impl的构造/析构、拷贝、移动等（现代C++用智能指针可减轻）。
- **调试略不便**：调试工具需要跟进指针才能看到实际数据。

# 实际项目中的PIMPL应用场景
- **大型库或框架的对外接口**（如Qt、Boost、标准库的实现类）
- **需要减小头文件依赖的模块化工程**
- **插件式架构**，需要稳定ABI

# 典型用法技巧
## 1. 拷贝和移动支持

如果你的类需要拷贝或移动，需要在Impl里实现相关操作，否则直接禁用拷贝构造和赋值：

```c++
MyClass(const MyClass&) = delete;
MyClass& operator=(const MyClass&) = delete;
```

或实现为：

```c++
MyClass(const MyClass& other) : pImpl(std::make_unique<Impl>(*other.pImpl)) {}
MyClass& operator=(const MyClass& other) {
    if (this != &other) {
        *pImpl = *other.pImpl;
    }
    return *this;
}
```

## 2. 使用智能指针

推荐用`std::unique_ptr<Impl>`，自动管理内存，避免资源泄漏。

## 3. 兼容API升级

当实现需要升级（如添加成员变量、换用新库等），只需改cpp文件，头文件和接口不变，用户无需重新编译。

- PIMPL=接口与实现分离，减少头文件暴露和依赖。
- 适用于工程规模较大、接口暴露稳定性要求高、编译耗时敏感场景。
- 现代C++（C++11及以后）推荐用`std::unique_ptr`或`std::shared_ptr`管理`Impl`。