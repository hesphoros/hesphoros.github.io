# Loophole

本文档介绍了 Boost.PFR 库中的两个基于 C++14 标准的回退反射引擎：**Loophole 引擎**（位于 `core14_loophole.hpp`）和 **Classic 引擎**（位于 `core14_classic.hpp`）。当编译器不支持 C++17 的结构化绑定（Structured Bindings）或 C++26 的新特性时，这些引擎会被激活。它们为库的其余部分提供了两个必需的底层原语：`tie_as_tuple` 和 `for_each_field_dispatcher`

此引擎利用了 **CWG 2118**（“通过友元注入进行状态元编程”），该技术最初由 Alexandr Poltavsky 描述。这项技术依赖于 C++ 的一条规则：在类模板内部定义的友元函数会在该类被实例化时注入到其外围命名空间中；并且，只要每个 `(T, N)` 对只定义一次，同一个友元函数就可以通过 `auto` 返回类型推导被多次声明。

> CWG 一致认为这种技术是不规范的，但尚未制定禁止机制的标准。编译器通常会接受它，但这并非绝对保证。



#### 共享基础：`offset_based_getter`


两个引擎都共享 `offset_based_getter<U, S>` 作为它们在运行时访问字段的机制。先理解它能让后续两个引擎的原理更容易理解。

`offset_based_getter` 类模板（位于 `offset_based_getter.hpp`）：

- `U` 是用户定义的结构体（例如 `MyStruct`）。
- `S` 是一个 `sequence_tuple::tuple<T0, T1, ...>`，它在内存布局上与 `U` 兼容。
- `tuple_of_aligned_storage_t<S>` 将每个 `Ti` 替换为 `internal_aligned_storage<sizeof(Ti), alignof(Ti)>`，创建一个与 `S`（因此也与 `U`）具有相同内存布局的、可进行常量表达式构造的元组（constexpr-constructible tuple）。
- `offset<idx>()` 通过在一个 `tuple_of_aligned_storage_t<S>` 的常量表达式实例中检查 `storage_` 指针，计算第 `idx` 个成员相对于第一个成员的字节偏移量。
- `get_pointer<idx>(U* u)` 通过 `reinterpret_cast<char*>` 将该偏移量应用到 `u` 的地址上，然后再转换回 `index_t<idx>*` 类型。

静态断言（Static assertions）确保 `sizeof(U) == sizeof(S)` 且 `alignof(U) == alignof(S)`。



# Loophole核心实体与数据流

tag<T,N> 会生成友元声明


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

**`fn_def_lref<T, U, N, B>` 与 `fn_def_rref<T, U, N, B>`**：当使用 `B = false` 实例化时，它们将 `loophole(tag<T,N>)` 定义为返回一个类型为 `U`（或对于右值引用则为 `U&&`）的值。`bool` 参数 `B` 通过一个 `ins` SFINAE 探测来检测之前的实例化；如果函数已定义（`B = true`），则该特化为空，以避免多重定义错误。

- `fn_def_lref` 处理左值引用类型和不可移动（non-movable）类型。
- `fn_def_rref` 处理仅可移动（move-only）类型。

**`loophole_ubiq_lref<T, N>` 与 `loophole_ubiq_rref<T, N>`**：每个都有一个模板化的转换操作符（`operator U&()` 或 `operator U&&()`）。当编译器在 `T` 的聚合初始化期间解析为字段类型 `U` 的转换时，转换操作符模板参数列表中的 `sizeof(fn_def_lref<T, U, N, ...>)` 表达式会实例化 `fn_def_lref`，从而注入友元定义.

当以 `B = false` 实例化时，这些定义会将 `loophole(tag<T,N>)` 定义为返回类型 `U` 的值（若是右值引用则为 `U&&`）。bool参数 `B` 通过一个内嵌的 SFINAE 探测来检测是否已被实例化；如果该函数已经定义（即 `B = true`），则特化版本为空，以避免多重定义错误。

# 捕获完整的类型列表

**`loophole_type_list_lref<T, std::index_sequence<I...>>`**

```c
struct loophole_type_list_lref<T, index_sequence<I...>>
    : sequence_tuple::tuple< decltype(T{ loophole_ubiq_lref<T,I>{}... }, 0) >
{
    using type = sequence_tuple::tuple< decltype(loophole(tag<T,I>{}))... >;
};
```

- 基类继承是一个技巧，用于强制评估聚合初始化表达式，从而触发所有友元注入。
- 之后，`decltype(loophole(tag<T,I>{}))` 会读回为每个字段索引 `I` 记录的类型。
- 结果类型 `type` 是一个包含所有字段类型的 `sequence_tuple::tuple`

**`loophole_type_list_selector`**：对于可拷贝构造（copy-constructible）的类型（常见情况），选择 `loophole_type_list_lref`；对于仅可移动（move-only）的类型，选择 `loophole_type_list_rref`。这是因为某些标准库类型（如 `std::vector<std::unique_ptr<int>>`）上的 `std::is_copy_constructible` 可能错误地为 `true`，因此左值路径包含一个运行时转换（cast）的变通方案。

# 组装引用元组

**`tie_as_tuple_loophole_impl(T& lvalue)`**

1. 计算 `fields_count<type>()` 以获取字段数量。
2. 使用 `loophole_type_list_selector` 获得 `tuple_type`（字段类型列表）。
3. 构造一个 `offset_based_getter<type, tuple_type>`。
4. 调用 `make_flat_tuple_of_references(lvalue, getter, size_t_<0>{}, size_t_<tuple_type::size_v>{})` 来生成左值引用的元组

**入口点（Entry Points）**

| 函数                                                         | 功能                                                    |
| :----------------------------------------------------------- | :------------------------------------------------------ |
| `tie_as_tuple(T& val)`                                       | 返回一个 `sequence_tuple`，其中包含每个字段的左值引用。 |
| `for_each_field_dispatcher(T& t, F&& f, index_sequence<I...>)` | 使用 `tie_as_tuple_loophole_impl(t)` 的结果调用 `f`。   |

两者都通过 `static_assert` 对 `std::is_union<T>` 进行检查。

# Loophole 引擎的限制

由于字段访问最终是通过 `offset_based_getter` 中的 `reinterpret_cast` 完成的，**Loophole 引擎无法在 `constexpr` 上下文中使用**。其他限制（来自测试黑名单）包括：

| 被列入黑名单的测试          | 原因                                          |
| :-------------------------- | :-------------------------------------------- |
| `constexpr_ops`             | `constexpr` 中不允许使用 `reinterpret_cast`   |
| `get_const_field`           | `boost::pfr::get` 在 `const` 字段上编译失败   |
| `optional_chrono`           | 在持有 `chrono` 类型的 `std::optional` 上失败 |
| `template_constructor`      | 聚合字段中的模板构造函数                      |
| `tie_anonymous_const_field` | `structure_tie` 在 `const` 字段上失败         |

# Reference

https://deepwiki.com/boostorg/pfr/4.4-c++14-engines:-loophole-and-classic