#  **Boost.PFR (Precise Flat Reflection / std::pfr)**

- **特点**：Boost 官方库，号称 **“零宏、零标记、零代码生成”** 的无侵入静态反射。

- **适用场景**：简单结构体（POD/Aggregate type）的遍历、比较、序列化、打印。

- **实现原理**：利用 C++17 结构化绑定（Structured Binding）与模板黑魔法推导字段数量和类型。

- **示例**：

  ```c++
  #include <iostream>
  #include <boost/pfr.hpp>
  
  struct User { std::string name; int age; };
  
  User u{"Alice", 20};
  // 编译期获取成员数量
  static_assert(boost::pfr::tuple_size_v<User> == 2);
  
  // 遍历打印所有字段值（无需任何注册代码）
  boost::pfr::for_each_field(u, [](const auto& field) {
      std::cout << field << "\n";
  });
  ```

