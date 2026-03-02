#ifndef __NOCOPYABLE_HPP__
#define __NOCOPYABLE_HPP__

#include <type_traits>  // for std::is_base_of

/**
 * @class NoCopyAble
 * @brief 一个禁止拷贝的基类，用于作为资源管理类的基类。
 *
 * @details
 * NoCopyAble 提供受保护的默认构造函数与析构函数，使其只能被继承而不能直接实例化（语义上）。
 * 通过将拷贝构造函数和拷贝赋值运算符声明为 delete，从语言层面禁止拷贝与赋值操作，
 * 防止派生类对象发生意外拷贝导致的资源冲突或重复释放问题。
 *
 * 使用示例：
 * - 将 NoCopyAble 作为互斥量、文件描述符、网络连接等资源管理类的基类，以确保这些对象不可拷贝。
 *
 * @note 构造函数与析构函数为受保护，以便仅允许继承，而不鼓励直接实例化。
 */

// 模拟 Boost 的 noncopyable 结构
namespace NoCopyAble_  // 保护从意外 ADL
{
#ifndef NOCOPYABLE_BASE_TOKEN_DEFINED
#define NOCOPYABLE_BASE_TOKEN_DEFINED

// NoCopyAble 从 base_token 派生，以启用类型特征检测
// 派生类是否从 NoCopyAble 派生，而无需 NoCopyAble 本身的定义。
//
// base_token 的定义是宏保护的，以便类型特征可以本地定义它
// 而无需包含此头文件，以避免对 Core 的依赖。

// 一个空的标记结构体,只用于继承链中的标识点
struct base_token {};

#endif // #ifndef NOCOPYABLE_BASE_TOKEN_DEFINED

    class NoCopyAble : base_token
    {
    protected:
        #if !defined(NO_CXX11_DEFAULTED_FUNCTIONS) && !defined(NO_CXX11_NON_PUBLIC_DEFAULTED_FUNCTIONS)
            constexpr NoCopyAble() = default;
            ~NoCopyAble() = default;

            // Move operations are allowed
            NoCopyAble(NoCopyAble&&) noexcept = default;
            NoCopyAble& operator=(NoCopyAble&&) noexcept = default;
        #else
            NoCopyAble() {}
            ~NoCopyAble() {}

            // Move operations are allowed
            NoCopyAble(NoCopyAble&&) noexcept {}
            NoCopyAble& operator=(NoCopyAble&&) noexcept { return *this; }
        #endif
        #if !defined(NO_CXX11_DELETED_FUNCTIONS)
            NoCopyAble(const NoCopyAble&) = delete;
            NoCopyAble& operator=(const NoCopyAble&) = delete;
        #else
    private:  // 强调以下成员是私有的
            NoCopyAble(const NoCopyAble&);
            NoCopyAble& operator=(const NoCopyAble&);
        #endif
    };
}

// 类型特征：检测一个类型是否继承自 NoCopyAble 
// 类型特性的工作原理: 
// - std::is_base_of<Base, Derived> 只需要base的声明(或者前置声明) 而不需要Base的完整定义
// - 通过编译器的继承信息在编译时检查关系,而不依赖运行时代码
// - NocopyAble_::base_token 提供了一个轻量级的标记基类,避免了对 NoCopyAble 完整定义的依赖
// - 这样可以在不引入 NoCopyAble 定义的情况下使用 is_NoCopyAble 类型特性
template<class T> struct is_NoCopyAble : std::is_base_of<NoCopyAble_::base_token, T>
{
};


// class MyClass : public NoCopyAble {};
// static_assert(is_NoCopyAble<MyClass>::value, "Must be non-copyable");

typedef NoCopyAble_::NoCopyAble NoCopyAble;

#endif // __NOCOPYABLE_HPP__