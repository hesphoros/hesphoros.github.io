
#ifndef __MUTEXLOCK_HPP__
#define __MUTEXLOCK_HPP__

#include <thread>
#include <mutex>
#include <assert.h>

#ifdef __linux__
#include <pthread.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#endif
#include <string>
#include <errno.h>
#include <stdio.h>



#include "NoCopyAble.hpp"


namespace CurrentThread {
    // __thread int t_cachedTid      = 0;
    // __thread char t_tidString[32]  {};
    // __thread int t_tidStringLength = 6;
    // __thread const char* t_threadName = "unknown";
    // // static_assert to ensure pid_t is the same as int
    // static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");

    inline int tid() {
        // simplified gettid implementation
        #ifdef __linux__
        return static_cast<int>(::syscall(SYS_gettid));
        #else
        return 0; // Windows fallback
        #endif
    }
} // namespace CurrentThread

/**
 * @class MutexLock
 * @brief 互斥锁类（占位/接口声明），不可拷贝。
 *
 * @details
 * MutexLock 继承自 NonCopyAble，表明互斥锁实例不可被拷贝或赋值。
 * 该类在当前头文件中仅作接口/语义声明；具体的成员函数（如 lock()/unlock()）、
 * 内部实现（例如使用 pthread_mutex_t、std::mutex 或平台特定原语）应在实现文件中补充。
 *
 * @see NonCopyAble
 * @note 请在实现中提供必要的线程安全操作与 RAII 辅助类（如 ScopedLock）以便安全使用。
 */
class MutexLock : public NoCopyAble {
public:
    MutexLock() : m_holder(0) {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    ~MutexLock() {
        assert(m_holder == 0); // 确保析构时锁未被持有
        pthread_mutex_destroy(&m_mutex);
    }

    bool isLockedByThisThread() const {
       return m_holder == (CurrentThread::tid());
    }

    // 仅供MutexLockGuard等类调用 严禁用户直接调用
    void lock() {
        // 两行顺序不可交换 
        // 先加锁，再记录持有者，防止竞态条件
        pthread_mutex_lock(&m_mutex);  
        m_holder = CurrentThread::tid();
    }

    // 仅供MutexLockGuard等类调用 严禁用户直接调用
    void unlock() {
        // 先清除持有者，再解锁，防止竞态条件
        // 确保在解锁前，其他线程无法看到错误的持有者信息
        // 两行顺序不可交换
        m_holder = 0;
        pthread_mutex_unlock(&m_mutex);

        // 如果顺序颠倒:
        // 线程 A 释放锁： 线程 A 执行 pthread_mutex_unlock(&m_mutex)。
        //
        // 线程 C 立即获得锁： 在极短的时间窗口内，另一个等待锁的线程 C 立即获取了锁，
        // 并且在它的 lock() 函数中执行了:
        // m_holder = C_ID;（线程 C 将 m_holder 设置为自己的 ID）。
        //
        // 线程 A 错误的清零操作： 线程 A 继续执行它的第二步:
        // m_holder = 0;（线程 A 将 线程 C 刚刚设置的有效 ID 覆盖为 0）。
    }

    pthread_mutex_t* getPthreadMutex() {
        return &m_mutex;
    }

    void assertLocked(){
        assert(isLockedByThisThread());
    }

private:

    pthread_mutex_t m_mutex;  // 互斥锁
    pid_t           m_holder; // 记录当前持有锁的线程ID
};

#endif // __MUTEXLOCK_HPP__