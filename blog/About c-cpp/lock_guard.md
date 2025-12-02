# std::lock_guard

```cpp

  /** @brief A simple scoped lock type.
   *
   * A lock_guard controls mutex ownership within a scope, releasing
   * ownership in the destructor.
   *
   * @headerfile mutex
   * @since C++11
   */
  template<typename _Mutex>
    class lock_guard
    {
    public:
      typedef _Mutex mutex_type;

      explicit lock_guard(mutex_type& __m) : _M_device(__m)
      { _M_device.lock(); }

      lock_guard(mutex_type& __m, adopt_lock_t) noexcept : _M_device(__m)
      { } // calling thread owns mutex

      ~lock_guard()
      { _M_device.unlock(); }

      lock_guard(const lock_guard&) = delete;
      lock_guard& operator=(const lock_guard&) = delete;

    private:
      mutex_type&  _M_device;
    };
```

> - 类`lock_guard`是一个互斥体封装器，它提供了一个方便的[RAII风格](https://cppreference.cn/w/cpp/language/raii "cpp/language/raii")机制，用于在作用域块的持续时间内拥有一个互斥体。
> - 当一个`lock_guard`对象被创建时，它会尝试获取它被赋予的互斥体的所有权。当控制离开创建`lock_guard`对象的作用域时，`lock_guard`会被销毁，互斥体也会被释放。
> - `lock_guard`类不可复制。


# example 
```cpp
#include <iostream>
#include <mutex>
#include <string_view>
#include <syncstream>
#include <thread>
 
volatile int g_i = 0;
std::mutex g_i_mutex;  // protects g_i
 
void safe_increment(int iterations)
{
    const std::lock_guard<std::mutex> lock(g_i_mutex);
    while (iterations-- > 0)
        g_i = g_i + 1;
    std::cout << "thread #" << std::this_thread::get_id() << ", g_i: " << g_i << '\n';
 
    // g_i_mutex is automatically released when lock goes out of scope
}
 
void unsafe_increment(int iterations)
{
    while (iterations-- > 0)
        g_i = g_i + 1;
    std::osyncstream(std::cout) << "thread #" << std::this_thread::get_id()
                                << ", g_i: " << g_i << '\n';
}
 
int main()
{
    auto test = [](std::string_view fun_name, auto fun)
    {
        g_i = 0;
        std::cout << fun_name << ":\nbefore, g_i: " << g_i << '\n';
        {
            std::jthread t1(fun, 1'000'000);
            std::jthread t2(fun, 1'000'000);
        }
        std::cout << "after, g_i: " << g_i << "\n\n";
    };
    test("safe_increment", safe_increment);
    test("unsafe_increment", unsafe_increment);
}
```

可能输出:

```bash
safe_increment:
before, g_i: 0
thread #140121493231360, g_i: 1000000
thread #140121484838656, g_i: 2000000
after, g_i: 2000000
 
unsafe_increment:
before, g_i: 0
thread #140121484838656, g_i: 1028945
thread #140121493231360, g_i: 1034337
after, g_i: 1034337
```



copy from : [std::lock_guard - cppreference.cn - C++参考手册](https://cppreference.cn/w/cpp/thread/lock_guard)