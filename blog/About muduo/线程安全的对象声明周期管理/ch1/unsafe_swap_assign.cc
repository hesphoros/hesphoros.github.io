#include <cstdint>
#include "MutexLockGuard.hpp"
class Counter {

public:
    Counter& operator=(const Counter& rhs);
    int64_t m_value;
    mutable MutexLock m_mutex;
};

void swap(Counter& a, Counter& b) {
    MutexLockGuard lock_a(a.m_mutex);
    MutexLockGuard lock_b(b.m_mutex);
    int64_t temp = a.m_value;
    a.m_value    = b.m_value;
    b.m_value    = temp;
}

// 这个swap函数存在死锁风险
// 假设有两个线程T1和T2同时调用swap函数，传入的参数顺序相反：
// 线程T1调用swap(counterA, counterB)
// 线程T2调用swap(counterB, counterA)
// 可能的执行顺序如下：
// 1. 线程T1执行到 MutexLockGuard lock_a(a.m_mutex); 成功获得 counterA 的锁
// 2. 线程T2执行到 MutexLockGuard lock_a(a.m_mutex); 成功获得 counterB 的锁
// 3. 线程T1继续执行，尝试获得 counterB 的锁，但此时锁已被线程T2持有，T1阻塞等待
// 4. 线程T2继续执行，尝试获得 counterA 的锁，但此时锁已被线程T1持有，T2阻塞等待
// 结果：线程T1等待线程T2释放 counterB 的锁，线程T2等待线程T1释放 counterA 的锁，形成死锁

Counter& Counter::operator=(const Counter& rhs) {
    if (this == &rhs) {
        return *this;
    }
    MutexLockGuard this_lock(this->m_mutex);
    MutexLockGuard rhs_lock(rhs.m_mutex);
    this->m_value = rhs.m_value;
    return *this;
}
// 这个赋值操作符重载函数同样存在死锁风险
// 假设有两个线程T1和T2同时调用赋值操作符，
// 传入的参数顺序相反：
// 线程T1执行 counterA = counterB;
// 线程T2执行 counterB = counterA;
// 可能的执行顺序如下：
// 1. 线程T1执行到 MutexLockGuard this_lock(this->m_mutex); 成功获得 counterA 的锁
// 2. 线程T2执行到 MutexLockGuard this_lock(this->m_mutex); 成功获得 counterB 的锁
// 3. 线程T1继续执行，尝试获得 counterB 的锁，但此时锁已被线程T2持有，T1阻塞等待
// 4. 线程T2继续执行，尝试获得 counterA 的锁，但此时锁已被线程T1持有，T2阻塞等待
// 结果：线程T1等待线程T2释放 counterB 的锁，线程T2等待线程T1释放 counterA 的锁，形成死锁


// 解决方法：比较mutex地址，始终给较小地址的mutex加锁
void safe_swap(Counter& a, Counter& b) {
    if (&a == &b) {
        return;
    }
    Counter* first  = &a;
    Counter* second = &b;
    if (std::addressof(a) > std::addressof(b)) {
        first  = &b;
        second = &a;
    }
    MutexLockGuard lock_first(first->m_mutex);
    MutexLockGuard lock_second(second->m_mutex);
    int64_t temp = a.m_value;
    a.m_value    = b.m_value;
    b.m_value    = temp;
}