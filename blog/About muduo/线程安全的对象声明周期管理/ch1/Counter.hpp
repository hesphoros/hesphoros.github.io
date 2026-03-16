#include <iostream>

#include "NoCopyAble.hpp"
#include "MutexLockGuard.hpp"

class Counter : public NoCopyAble {

    //copy-ctor and assignment should be privade by default for class
public:
    Counter() : m_count(0) {}

    int64_t value() const {
        // lock 的析构会晚于返回对象的构造 所以有效保护这个共享数据
        MutexLockGuard lock(m_mutex);
        return m_count;
    }
    
    int64_t getAndIncrement() {
        MutexLockGuard lock(m_mutex);
        int64_t ret = m_count++;
        return ret;
    }

private:
    int64_t           m_count;
    mutable MutexLock m_mutex;
// In a real world, atomic<int64_t> m_count; would be better
};
