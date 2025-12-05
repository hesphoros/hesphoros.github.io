# shared_ptr技术与陷阱

# bind陷阱

在`boost::bind` 和 `std::bind` 中,在默认情况下会对所有传递给它的**实参**进行**值拷贝（copy）**。

- **`shared_ptr` 的行为：** 当 `shared_ptr` 被拷贝时，它的**引用计数**会增加

```cpp
shared_ptr<Foo> pFoo(new Foo);
boost::function<void()> func = boost::bind(&Foo::doit, pFoo);
```

`boost::bind` 将 `pFoo` **拷贝**了一份，这份拷贝被存储在了最终的 `boost::function` 对象 `func` 内部。只要 `func` 对象存在，它内部的 `shared_ptr` 拷贝就存在，`Foo` 对象的引用计数就至少为 1，因此 `Foo` 对象就不会被销毁，从而**延长了它的生命周期*

std::bind

```cpp
#include <iostream>
#include <functional>
#include <memory>

class Foo {
public:
    void doit() { std::cout << "Foo::doit called." << std::endl; }
    ~Foo() { std::cout << "Foo destroyed." << std::endl; }
};

void run_std_bind_issue() {
    std::cout << "--- std::bind 演示 ---\n";
    
    // 1. 创建原始 shared_ptr (引用计数 = 1)
    std::shared_ptr<Foo> pFoo(new Foo);
    std::cout << "原始 pFoo 引用计数: " << pFoo.use_count() << "\n"; // 1

    // 2. 使用 std::bind 绑定成员函数和对象实例
    // std::bind 会拷贝 pFoo，将其存储在新函数对象 f 内部
    auto f = std::bind(&Foo::doit, pFoo); 
    std::cout << "绑定后 pFoo 引用计数: " << pFoo.use_count() << "\n"; // 2 (原始 pFoo + f 内部的拷贝)

    // 原始 pFoo 离开作用域，引用计数 -1 (变为 1)
} // f 离开作用域，f 内部的 shared_ptr 销毁，引用计数 -1 (变为 0)，Foo 对象被销毁。

// 演示函数结束时，Foo 对象才会被销毁

```

## 函数传参

因为要修改引用计数(而且拷贝的时候通常需要加锁), shared_ptr的拷贝开销比拷贝原始指针要高, 多数情况下可以以 const reference 进行传递.

一个线程只需要在最外层函数中有一个实体对象, 之后都可以以用const reference 来使用这个shared_ptr 

```cpp

void save(const shared_ptr<Foo>& pFoo); // pass by const reference
void validateAccount(const Foo& foo);

bool validate(const shard_ptr<Foo>& pFoo){
	validateAccount(*pFoo);
}

```

通常情况下,我们可以传常引用

```cpp
void onMessage(const string& msg) {
	shared_ptr<Foo> pFoo(new Foo(msg)); // 只需要在最外层持有一个实体 安全不成问题
	if(validate(pFoo)) { // no cpoy pFoo
		save(pFoo);
	}
} 

```

析构函数在创建时被捕获 这意味着:

- 虚析构不再必须
- shared_ptr<\void\> 可以持有任何对象,而且能安全的释放
- shared_ptr 对象可以安全的跨越模块边界,比如从DLL里返回,而不会造成从模块A分配的内存在模块B里被释放的这种错误
- 二进制兼容性
	- 即使Foo对象的大小变了,那么旧的客户代码仍然可以使用新的动态库而无需重新编译(前提是Foo的头文件不出现访问对象的成员的inline函数,并且Foo对象的动态库中的Factory构造 返回其shared_ptr)
- 析构动作可定制
