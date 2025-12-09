#ifndef __SIGNAL_SLOT_TRIVIAL_H__
#define __SIGNAL_SLOT_TRIVIAL_H__

#include <memory>
#include <vector>
#include <functional>

template <typename Signature>
class SignalTrivial;

/**
 * @brief SignalTrivial 类模板的特化版本，处理函数签名 RET(ARGS...)
 * @tparam RET 函数返回类型
 * @tparam ARGS 函数参数类型包
 * @example 
 *  * SignalTrivial<void(int)> signal;
 *  * signal.connect([](int x) { std::cout << x << std::endl; });
 *  * signal.call(42);
 */
template <typename RET, typename... ARGS>
class SignalTrivial<RET(ARGS...)>
{

public:
    using Functor = std::function<void (ARGS...)>;

    void connect(Functor&& slot)
    {
        // 使用std::forward完美转发
        slots_.emplace_back(std::forward<Functor>(slot));
    }

    void call(ARGS&&... args)
    {
        for (const Functor& slot : slots_)
        {
            slot(std::forward<ARGS>(args)...);
        }
    }
private:
    std::vector<Functor> slots_;
};

#endif //__SIGNAL_SLOT_TRIVIAL_H__