#ifndef __SIGNAL_SLOT_HPP__
#define __SIGNAL_SLOT_HPP__

#include "MutexLock.hpp"
#include "MutexLockGuard.hpp"

#include <assert.h>
#include <functional>
#include <memory>
#include <vector>

template <typename Callback>
struct SlotImpl;

template <typename Callback>
struct SignalImpl : NoCopyAble
{
    // 使用weak_ptr 避免循环引用
    // Slot通常由外部对象持有(std::shared_ptr)
    //  如果 SignalImpl 用 shared_ptr 存储槽，会形成循环引用（Signal → Slot → Signal），导致内存泄漏。
    // 使用weak_ptr:
    // - 信号不再拥有槽的声明周期,槽的生命周期由外部控制
    // - 当槽被销毁时,weak_ptr 会自动失效,避免悬空指针
    // - 避免了循环引用
    typedef std::vector<std::weak_ptr<SlotImpl<Callback>>> SlotList;

    // Copy-on-write 
    // 在多线程环境中, 如果一个线程正在遍历slots,另一个线修改它(连接/断开) 会发生race-condition
    // solution:
    // 解决：将 SlotList 包装在 shared_ptr 中。修改时，创建一个新的 SlotList 副本，
    // 修改副本，然后原子替换 slots_ 指针。正在遍历旧列表的线程不受影响（旧列表仍然有效），新线程看到新列表。
    std::shared_ptr<SlotList>       slots_;

    SignalImpl()
        : slots_(std::make_shared<SlotList>())
    {}

    //     线程A（读取）                    线程B（写入）
    // ─────────────────────────────────────────────────────
    // │                                │
    // │ local = slots_;  ─────────────►│  // A 获取 slots_ 副本
    // │ (ref_count = 2)                │
    // │                                │ MutexLockGuard lock(mutex_);
    // │                                │ copyOnWrite();
    // │                                │   ├─ !slots_.unique() → true
    // │                                │   ├─ slots_.reset(new SlotList(*slots_))
    // │                                │   │   ┌─────────────┐
    // │                                │   │   │ 旧 SlotList │ ← A 仍在使用
    // │                                │   │   └─────────────┘
    // │                                │   │   ┌─────────────┐
    // │                                │   │   │ 新 SlotList │ ← B 修改这个
    // │                                │   │   └─────────────┘
    // │                                │ slots_->push_back(newSlot);
    // │ for slot in local:             │
    // │   slot.lock()->call();         │  // A 安全遍历旧列表
    // │                                │
    void copyOnWrite() {
        mutex_.assertLocked();  // 保证只有一个线程能执行写操作
        if(!slots_.unique()) {  // 执行条件: 当有其他的线程指向同一个SlotList 即,当 slots_ 的引用计数 > 1
            // 复制旧列表：new SlotList(*slots_) 创建 slots_ 指向的 SlotList 的副本。
            // 替换指针：reset() 让 slots_ 指向新副本。
            // 旧列表仍然存在（其他线程持有的 shared_ptr 指向它），遍历不受影响。
            // 只有当当前没有其他shared_ptr引用时，才进行复制

            // 执行前
            // ┌─────────────────────────────────────────────────────────┐
            // │ slots_ ──────────────────┐                              │
            // │                          ▼                              │
            // │ local (线程A) ─────────► SlotList (旧)  ref_count = 2   │
            // │                          [slot1, slot2, slot3]          │
            // └─────────────────────────────────────────────────────────┘

            // 执行后：
            // ┌─────────────────────────────────────────────────────────┐
            // │ local (线程A) ─────────► SlotList (旧)  ref_count = 1   │
            // │                          [slot1, slot2, slot3]          │
            // │                                                         │
            // │ slots_ ─────────────────► SlotList (新)  ref_count = 1  │
            // │                          [slot1, slot2, slot3] (副本)   │
            // └─────────────────────────────────────────────────────────┘
            slots_.reset(new SlotList(*slots_));
        }
        assert(slots_.unique()); // 现在 slots_ 是唯一引用
    }

    /**
     * @brief 清理已失效的槽（weak_ptr 指向的对象已销毁）
     * 
     * @details
     * 遍历 slots_ 列表，移除所有已过期（expired）的 weak_ptr。
     * 这是一种"懒清理"策略：槽销毁时不主动通知信号，而是在适当时机统一清理。
     */
    void clean() {
        MutexLockGuard lock(mutex_);  // 1. 加锁，保证线程安全
        copyOnWrite();                 // 2. 写时复制，确保独占访问（不影响正在读取的线程）
        
        SlotList& list(*slots_);       // 3. 获取 SlotList 的引用，方便操作
        auto it = list.begin();        // 4. 迭代器指向列表开头
        
        while(it != list.end())        // 5. 遍历整个列表
        {
            if (it->expired())         // 6. 检查 weak_ptr 是否已失效
            {                          //    （指向的 shared_ptr 已销毁）
                it = list.erase(it);   // 7. 失效：从列表中移除，erase() 返回下一个有效迭代器
            }
            else
            {
                ++it;                  // 8. 未失效：移动到下一个元素
            }
        }
    }

    MutexLock mutex_;

};

template <typename Callback>
struct SlotImpl : NoCopyAble
{
    typedef SignalImpl<Callback> Data;

    std::weak_ptr<Data>     data_;      // 指向信号实现的弱指针
    Callback                callback_;  // 回调槽函数
    std::weak_ptr<void>     tie_;       // 绑定对象的弱指针
    bool                    tied_;      // 是否绑定了对象

    SlotImpl(const std::shared_ptr<Data>& data, const Callback&& cb)
        : data_(data)
        , callback_(cb)
        , tie_()
        , tied_(false)
    {};

    SlotImpl(const std::shared_ptr<Data>& data, const Callback&& cb,
        const std::shared_ptr<void>& tie)
        : data_(data)
        , callback_(cb)
        , tie_(tie)
        , tied_(true)
    {}
    
    ~SlotImpl() {
        // 此处创建一个data的shared_ptr副本,确保在调用clean时Data对象仍然有效
        std::shared_ptr<Data> data(data_.lock());
        if (data) {
            data->clean();
        }
    }
    
};            


/// This is the handle for a slot
/// 
/// 只要这个返回的 Slot 对象（以及它的任何副本）还活着，信号和槽的连接就会保持有效。

template <typename Signalture>
class Signal;
/// partial specialization for function types

template <typename RET, typename... ARGS>
class Signal<RET(ARGS...)> : NoCopyAble
{   
public:
    /***
     * @param RET 函数返回类型
     * @param ARGS 函数参数类型包
     * @brief 回调函数的类型
     */
    typedef std::function<RET(ARGS...)>               Callback;
    
    /**
     * @brief 信号内部实现类型
     */
    typedef SignalImpl<Callback>                 SignalImplType; 
    /**
     * @brief 槽内部实现类型
     */
    typedef SlotImpl<Callback>                     SlotImplType;

    Signal()
        : impl_(std::make_shared<SignalImplType>())
    {}
    
    ~Signal() {}

    /**
     * @brief 连接一个槽到信号
     * 
     * @param slot 要连接的槽函数
     * @return std::shared_ptr<void> 返回一个槽的句柄，只要这个句柄还活着，连接就有效
     */
    std::shared_ptr<void> connect(Callback&& slot) {
        std::shared_ptr<SlotImplType> slot_impl(
            std::make_shared<SlotImplType>(impl_, 
                std::forward<Callback>(slot))
        );
        add(slot_impl);
        return slot_impl;
    }

    /**
     * @brief 连接到槽
     * @param slot 要连接的槽函数
     * @param tie 绑定的对象，用于管理槽的生命周期
     */
    std::shared_ptr<void> connect(Callback&& slot,
        const std::shared_ptr<void>& tie) 
    {
        std::shared_ptr<SlotImplType> slot_impl(
            std::make_shared<SlotImplType>(impl_, 
                std::forward<Callback>(slot), tie)
        );
        add(slot_impl);
        return slot_impl;
    }

    void call(ARGS&&... args)
    {
        SignalImplType& impl(*impl_);
        // 获取当前槽列表的副本，避免在遍历时被修改
        std::shared_ptr<typename SignalImplType::SlotList> slots;
        {
            MutexLockGuard lock(impl.mutex_);
            slots = impl.slots_;
        }
        // 遍历槽列表，调用每个槽
        typename SignalImplType::SlotList& slotList(*slots);
        for (
            typename SignalImplType::SlotList::const_iterator it = slotList.begin();
            it != slotList.end();
            ++it) 
        {
            std::shared_ptr<SlotImplType> slot = it->lock();
            if (slot)
            {
                std::shared_ptr<void> guard; // 用于保持绑定对象的生命周期
                if(slot->tied_) {
                    // 如果槽绑定了对象，尝试获取绑定对象的 shared_ptr
                    guard = slot->tie_.lock();
                    if (guard) {
                        // 绑定对象仍然存在，调用槽函数
                        slot->callback_(std::forward<ARGS>(args)...);
                    }
                }  else {
                    // 未绑定对象，直接调用槽函数
                    slot->callback_(std::forward<ARGS>(args)...);
                }
            }

        } 
    }

private:
    std::shared_ptr<SignalImplType>   impl_;  // 信号的实现

    /**
     * @brief 添加槽到信号
     * @param slot 要添加的槽
     */
    void add(const std::shared_ptr<SlotImplType>& slot) 
    {
        MutexLockGuard lock(impl_->mutex_);
        impl_->copyOnWrite();  // 确保独占访问
        impl_->slots_->emplace_back(slot); // 添加新的槽
    }
};


#endif // ! __SIGNAL_SLOT_HPP__