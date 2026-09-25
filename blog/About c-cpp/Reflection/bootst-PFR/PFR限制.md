Boost.PFR 的反射功能存在一些限制，这些限制取决于 C++ 标准和编译器能力

- 静态变量会被忽略
- T 必须是可在没有基类的情况下进行聚合初始化的
- 如果 T 包含 C 数组，则反射结果可能会因 C++ 版本和库配置的不同而有所差异
- 如果 **BOOST_PFR_USE_CPP17 == 0**，则有以下附加限制
    
    - 任何成员字段都不应具有接受一个参数的模板构造函数。
    - 如果 **BOOST_PFR_USE_LOOPHOLE == 0**，则有以下附加限制
        
        - T 必须是可 constexpr 聚合初始化的，且其所有字段必须是可 constexpr 默认构造的
        - `[boost::pfr::get]`[boost::pfr::structure_to_tuple](https://boost.ac.cn/doc/libs/latest/doc/html/doxygen/reference_section_of_pfr/core_8hpp_1afa68927e3e0e191200fad450e79bb389.html "Function template structure_to_tuple")`、`boost::pfr::structure_tie`、`[boost::pfr::tuple_element](https://boost.ac.cn/doc/libs/latest/doc/html/reference_section_of_pfr.html#doxygen.reference_section_of_pfr.core_8hpp "Header <boost/pfr/core.hpp>")` 要求 T 必须是仅包含内置类型的 POD 类型。
        
    
Boost.PFR 的字段名提取功能存在一些限制，这些限制取决于 C++ 标准和编译器能力

- T 应该可以像 `extern T t;` 一样使用，即具有非内部链接（non-internal linkage）。