字段反射与引用计数
字段数量检测与获取成员引用:https://boost.ac.cn/doc/libs/latest/doc/html/boost_pfr/how_it_works.html

- **BOOST_PFR_USE_CPP26_REFLECTION == 0 && BOOST_PFR_USE_CPP26 == 0 && BOOST_PFR_USE_CPP17 == 1**:
    
    1. 在编译期：使用结构化绑定将类型 `T` 分解为已知数量的字段

- **BOOST_PFR_USE_CPP26_REFLECTION == 0 && BOOST_PFR_USE_CPP26 == 0 && BOOST_PFR_USE_CPP17 == 0 && BOOST_PFR_USE_LOOPHOLE == 1**:
	    
    1. 在编译期：使用聚合初始化检测用户提供结构体中的字段数量
    2. 在编译期：创建一个可转换为任何类型的结构体，并记录在用户提供结构体的聚合初始化过程中它被转换成的类型
    3. 在编译期：利用前几个步骤中获得的信息，创建一个与用户提供结构体具有完全相同布局的 tuple
    4. 在编译期：使用上一步中的 tuple，查找用户提供结构体中每个字段的偏移量
    5. 在运行期：已知结构体地址和每个字段的偏移量，获取指向每个字段的指针
    6. 在运行期：返回字段引用的 tuple => 该结构体可以使用所有的 tuple 方法
    
1. 在编译期
    - 在 `constexpr` 函数中获取类型为 `T` 的对象的成员引用
    - 将上一步中的引用作为模板参数传递给一个带有 `template <auto member_ptr>` 的 `constexpr` 函数。该函数返回 `__PRETTY_FUNCTION__` 或其他特定于编译器的宏，用于打印完整的函数名以及模板参数。
    - 上一步返回的值包含成员名称（[godbolt 示例](https://godbolt.org/z/K4aWdcE9G)）。对该值进行一些特定于编译器的解析，并生成一个仅包含成员名称的 `std::string_view`。
2. 在运行期：返回包含成员名称的 `std::string_view`。