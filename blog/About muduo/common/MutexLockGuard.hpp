/**
 * @class MutexLockGuard
 * @brief 基于 RAII 的互斥锁管理类（在构造时上锁，析构时解锁）。
 *
 * @details
 * 该类在构造时对传入的 MutexLock 引用调用 lock()，在析构时调用 unlock()。
 * 通过作用域控制互斥锁的加解锁，确保在异常抛出或提前返回的情况下也能正确释放锁，
 * 提供异常安全的临界区保护。类继承自 NonCopyable，禁止拷贝以保证锁的唯一所有权。
 *
 * 典型用法：
 * @code
 * MutexLock mutex;
 * {
 *     MutexLockGuard guard(mutex); // 进入临界区：已加锁
 *     // 临界区代码
 * } // 离开作用域时自动解锁
 * @endcode
 *
 * @param mutex 引用类型，指向要管理的 MutexLock 对象。调用者必须确保该 mutex 的生存期
 *              长于本 Guard 实例（即在 Guard 销毁前 mutex 不被销毁）。
 *
 * @note 构造函数为 explicit，防止意外的隐式类型转换导致的错误加锁。
 *
 * @warning
 * - 不要拷贝或赋值本类实例（由 NonCopyable 强制），否则会破坏锁的语义。
 * - 避免对同一把互斥锁使用多个独立的 Guard（除非底层 MutexLock 明确支持可重入/递归锁）。
 * - 请确保传入的 MutexLock 在 Guard 生命周期内有效，避免悬空引用。
 *
 * @see MutexLock, NonCopyable
 */

#ifndef __MUTEXLOCKGUARD_HPP__
#define __MUTEXLOCKGUARD_HPP__

#include "MutexLock.hpp"
#include "NoCopyAble.hpp"

/**
 * @def MutexLockGuard(x)
 * @brief 辅助宏：在遗漏变量名时触发编译期静态断言，避免误用类名作为变量声明。
 *
 * @details
 * 该宏用于捕获用户在使用 MutexLockGuard 时仅写出类型而未提供变量名的错误写法（例如
 * 误写为 "MutexLockGuard(mutex);"，这会被解析为函数声明而不是一个局部变量）。当用户
 * 错误使用该形式时，宏展开将触发 static_assert(false, "Missing mutex guard variable name")，
 * 使编译失败并给出明确提示。
 *
 * @warning 该宏仅用于帮助捕获常见的语法误用，不应作为正常代码路径的一部分。正确的用法是：
 * @code
 * MutexLockGuard guard(mutex);
 * @endcode
 */
class MutexLockGuard : public NoCopyAble {
public:
    explicit MutexLockGuard(MutexLock& mutex) : m_mutex(mutex) {
        m_mutex.lock();
    }

    ~MutexLockGuard() {
        m_mutex.unlock();
    }

private:
    MutexLock& m_mutex;
};

#define MutexLockGuard(x) static_assert(false,"Missing mutex gurad variable name")

#endif // __MUTEXLOCKGUARD_HPP__
