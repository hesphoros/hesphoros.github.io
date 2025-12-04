#ifndef __NOCOPYABLE_HPP__
#define __NOCOPYABLE_HPP__

/**
 * @class NonCopyAble
 * @brief 一个禁止拷贝的基类，用于作为资源管理类的基类。
 *
 * @details
 * NonCopyAble 提供受保护的默认构造函数与析构函数，使其只能被继承而不能直接实例化（语义上）。
 * 通过将拷贝构造函数和拷贝赋值运算符声明为 delete，从语言层面禁止拷贝与赋值操作，
 * 防止派生类对象发生意外拷贝导致的资源冲突或重复释放问题。
 *
 * 使用示例：
 * - 将 NonCopyAble 作为互斥量、文件描述符、网络连接等资源管理类的基类，以确保这些对象不可拷贝。
 *
 * @note 构造函数与析构函数为受保护，以便仅允许继承，而不鼓励直接实例化。
 */
class NonCopyAble {
protected:
    NonCopyAble()  = default;
    ~NonCopyAble() = default;

    NonCopyAble(const NonCopyAble&) = delete;
    NonCopyAble& operator=(const NonCopyAble&) = delete;
};

#endif // __NOCOPYABLE_HPP__