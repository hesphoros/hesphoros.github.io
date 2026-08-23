# Loophole

本文档介绍了 Boost.PFR 库中的两个基于 C++14 标准的回退反射引擎：**Loophole 引擎**（位于 `core14_loophole.hpp`）和 **Classic 引擎**（位于 `core14_classic.hpp`）。当编译器不支持 C++17 的结构化绑定（Structured Bindings）或 C++26 的新特性时，这些引擎会被激活。它们为库的其余部分提供了两个必需的底层原语：`tie_as_tuple` 和 `for_each_field_dispatcher`

该引擎利用了 CWG 2118（“通过友元注入实现有状态元编程”），该技术最初由 Alexandr Poltavsky 提出。

它使用 C++ 的一条规则：类模板中的友元函数定义会在类实例化时被注入到封闭的命名空间中，并且同一个友元函数可以通过`auto`返回类型推导被多次声明，只要它只包含一个元素即可。定义每对 `(T, N)`一次。

**注：** CWG 一致认为这种技术是不规范的，但尚未制定禁止机制的标准。编译器通常会接受它，但这并非绝对保证。



#### 共享基础：`offset_based_getter`



两个引擎都共享 `offset_based_getter<U, S>` 作为它们在运行时访问字段的机制。先理解它能让后续两个引擎的原理更容易理解。

`offset_based_getter` 类模板（位于 `offset_based_getter.hpp`）：

- `U` 是用户定义的结构体（例如 `MyStruct`）。
- `S` 是一个 `sequence_tuple::tuple<T0, T1, ...>`，它在内存布局上与 `U` 兼容。
- `tuple_of_aligned_storage_t<S>` 将每个 `Ti` 替换为 `internal_aligned_storage<sizeof(Ti), alignof(Ti)>`，创建一个与 `S`（因此也与 `U`）具有相同内存布局的、可进行常量表达式构造的元组（constexpr-constructible tuple）。
- `offset<idx>()` 通过在一个 `tuple_of_aligned_storage_t<S>` 的常量表达式实例中检查 `storage_` 指针，计算第 `idx` 个成员相对于第一个成员的字节偏移量。
- `get_pointer<idx>(U* u)` 通过 `reinterpret_cast<char*>` 将该偏移量应用到 `u` 的地址上，然后再转换回 `index_t<idx>*` 类型。

静态断言（Static assertions）确保 `sizeof(U) == sizeof(S)` 且 `alignof(U) == alignof(S)`。



# 实现思路

tag<T,N> 会生成友元声明
这里有两种类型：

- 一种是使用 auto 返回类型的，这是我们后续读取类型的方式。
- 第二种用于检测实例化，如果没有它，我们将会得到多重定义。


```c
// tag<T,N> generates friend declarations and helps with overload resolution.
// There are two types: one with the auto return type, which is the way we read types later.
// The second one is used in the detection of instantiations without which we'd get multiple
// definitions.

template <class T, std::size_t N>
struct tag {
    // 声明一个友元函数，返回类型待推导
    friend auto loophole(tag<T,N>);
};
```

**声明**（但不定义）一个具有 `auto` 返回类型的友元函数 `loophole(tag<T,N>)`。这就是存储类型信息的“槽位”。



```c
// The definitions of friend functions.
template <class T, class U, std::size_t N, bool B>
struct fn_def_lref {
    friend auto loophole(tag<T,N>) {
        // Standard Library containers do not SFINAE on invalid copy constructor. Because of that std::vector<std::unique_ptr<int>> reports that it is copyable,
        // which leads to an instantiation error at this place.
        //
        // To workaround the issue, we check that the type U is movable, and move it in that case.
        using no_extents_t = std::remove_all_extents_t<U>;
        return static_cast< std::conditional_t<std::is_move_constructible<no_extents_t>::value, no_extents_t&&, no_extents_t&> >(
            boost::pfr::detail::unsafe_declval<no_extents_t&>()
        );
    }
};
template <class T, class U, std::size_t N, bool B>
struct fn_def_rref {
    friend auto loophole(tag<T,N>) { return std::move(boost::pfr::detail::unsafe_declval< std::remove_all_extents_t<U>& >()); }
};


// Those specializations are to avoid multiple definition errors.
template <class T, class U, std::size_t N>
struct fn_def_lref<T, U, N, true> {};

template <class T, class U, std::size_t N>
struct fn_def_rref<T, U, N, true> {};

```

 标准库容器在复制构造函数无效时不会进行 SFINAE 处理。因此 `std::vector<std::unique_ptr<int>>` 会报告它是可复制的，
这会导致在此处产生实例化错误。
 为了解决这个问题，我们检查类型 `U` 是否可移动，并在可移动的情况下对其进行移动操作。



当以 `B = false` 实例化时，这些定义会将 `loophole(tag<T,N>)` 定义为返回类型 `U` 的值（若是右值引用则为 `U&&`）。bool参数 `B` 通过一个内嵌的 SFINAE 探测来检测是否已被实例化；如果该函数已经定义（即 `B = true`），则特化版本为空，以避免多重定义错误。



# Reference

https://deepwiki.com/boostorg/pfr/4.4-c++14-engines:-loophole-and-classic