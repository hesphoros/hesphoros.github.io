#ifndef __SIGNAL_SLOT_TRIVIAL_H__
#define __SIGNAL_SLOT_TRIVIAL_H__

#include <memory>
#include <vector>
#include <functional>

template <typename Signature>
class SignalTrivial;

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