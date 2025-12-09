// 此代码演示使用全局的`facade`来代理Foo类型对象访问
// 见7. 实现线程安全的对象回调和析构
/*****************************************************************************
*  @file     Facade.cpp                                                
*  @brief    全局Facade代理对象访问的示例代码                                                  
*  @details  详细描述                                                 
*                                                                           
*  @author   hesphoros                                                
*  @email    hesphoros@gmail.com                                 
*  @version  1.0.0.1                                                       
*  @date     2025/12/09             
*  @license  GNU General Public License (GPL)                              
*---------------------------------------------------------------------------*
*  Remark         : 说明备注                                          
*---------------------------------------------------------------------------*
*  Change History :                                                        
*  <Date>     | <Version> | <Author>       | <Description>                 
*  2025/12/09 | 1.0.0.1   | hesphoros      | Create file           
*****************************************************************************/

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>

// - class Foo 是实际的对象
// - FooHandle 是对象的句柄/ID
// - FooManager是充当全局Facade的单例

using FooHandle = size_t;

/**
 * @brief 实际的业务对象,但是它的方法不会直接被外部进行调用
 * @detial 此处演示一个简单的Foo类
 */
class Foo {
private:
    int m_value;
public:
    Foo(int value) : m_value(value) {
        std::cout << "Foo created with value: " << m_value << std::endl;
    }

    ~Foo() {
        std::cout << "Foo with value " << m_value << " destroyed." << std::endl;
    }

    void doWork(int delta) {
        m_value += delta;
        std::cout << "Foo value updated to: " << m_value << std::endl;

    }
};




/**
 * @brief 全局的Facade,负责管理所有Foo对象的生命周期和访问权限
 * @details 使用一个全局锁和一个map来实现线程安全的chek-out/check-in机制
 *          使用单例简化
 */
class FooManager {
private:
    std::mutex                             m_global_mtx;  // 全局互斥锁     
    std::map<FooHandle, std::unique_ptr<Foo>> m_foo_map;  // 存储所有活跃的 Foo 对象
    std::atomic<FooHandle>                m_next_handle{1}; // 下一个可用的句柄
    FooManager() = default; // 私有构造函数，防止外部实例化
public:
    static FooManager& getInstance() {
        static FooManager instance;
        return instance;
    }

    /**
     * @brief 创建一个新的Foo对象，并返回其句柄
     */
    FooHandle createFoo(int value) {
        std::lock_guard<std::mutex> lock(m_global_mtx);
        FooHandle handle = m_next_handle++;
        m_foo_map[handle] = std::make_unique<Foo>(value);
        return handle;
    }

    /**
     * @brief 析构指定句柄的Foo对象
     */
    void destroyFoo(FooHandle handle) {
        std::lock_guard<std::mutex> lock(m_global_mtx); // 锁住整个操作
        // 实际的析构/清理在这里发生
        if (m_foo_map.count(handle)) {
            m_foo_map.erase(handle);
        }
    }

    // 代理访问
    // 这是核心方法: 模拟对Foo成员函数的线程安全调用
    void callDoWork(FooHandle handle, int delta) {
        std::cout << "Thread " << std::this_thread::get_id() << " trying to CHECK-OUT Foo " << handle << std::endl;
        
        // **CHECK-OUT:** 获取全局锁，找到对象
        // 关键点：这个锁保护了所有 Foo 对象的访问，即使它们不相关。
        std::lock_guard<std::mutex> lock(m_global_mtx);
        
        std::cout << "Thread " << std::this_thread::get_id() << " CHECKED-OUT Foo " << handle << " (Lock Acquired)" << std::endl;

        auto it = m_foo_map.find(handle);
        if (it != m_foo_map.end()) {
            // 访问和操作对象
            it->second->doWork(delta); 
            // 访问完成后，锁在 std::lock_guard 析构时自动释放 (CHECK-IN)
        } else {
            std::cout << "Error: Foo object with handle " << handle << " not found." << std::endl;
        }

        std::cout << "Thread " << std::this_thread::get_id() << " CHECKED-IN Foo " << handle << " (Lock Released)" << std::endl;
    }
    // 禁用拷贝和赋值
    FooManager(const FooManager&) = delete;
    FooManager& operator=(const FooManager&) = delete;

};  


// 模拟多线程访问
void worker(FooHandle handle, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        FooManager::getInstance().callDoWork(handle, 1);
        // 模拟一些其他工作，但因为全局锁，这仍然是串行的
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    std::cout << "--- 1. 创建两个 Foo 对象 ---" << std::endl;
    FooHandle h1 = FooManager::getInstance().createFoo(100);
    FooHandle h2 = FooManager::getInstance().createFoo(200);

    std::cout << "\n--- 2. 两个线程访问不同对象 ---" << std::endl;
    // 线程 A 访问对象 h1
    std::thread t1(worker, h1, 3);
    // 线程 B 访问对象 h2
    // **注意：** 尽管 t1 和 t2 访问的是不同的对象 (h1 和 h2)，
    // 但因为它们都必须通过 FooManager 的 **同一个全局锁** 来 CHECK-OUT/CHECK-IN，
    // 它们的访问实际上是**串行**的。
    std::thread t2(worker, h2, 3);

    t1.join();
    t2.join();

    std::cout << "\n--- 3. 销毁对象 ---" << std::endl;
    FooManager::getInstance().destroyFoo(h1);
    FooManager::getInstance().destroyFoo(h2);

    return 0;
}

// 如果你运行这段代码，你会看到线程 t1 和线程 t2 访问 Foo 对象的操作是 交替 进行的，而不是 并发 进行的
// 缺点: 
// 为了使 Façade 线程安全，必须使用 互斥锁 (Mutex)。
// 这样会导致访问不同 Foo 对象的两个线程也可能使用 同一个锁，
// 将原本可以 并行执行 的函数变成了 串行执行，损失了 多核优势。

// 优化思路:
// 为了降低锁竞争可以考虑如下改进:
// 分桶加锁(Bucketing): 不要使用一个全局锁,而是将m_foo_map容器分为多个buckets(比如std::verctor<std::map<FooHandle,...>)
// Hash句柄: 根据FooHandle的值进行hash,决定他属于哪个bucket
// 细粒度锁: 每个bucket拥有自己的互斥锁,这样只有当两个线程同时访问同一个bucket中的对象时才会发生锁竞争