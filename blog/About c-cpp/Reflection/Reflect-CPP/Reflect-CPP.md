# Reflect-CPP

- **特点**：现代 C++20/23 静态反射与序列化库，极其流行。

- **功能**：自动支持 JSON、XML、YAML、CBOR、TOML 的互相转换。

- **示例**：

  ```c++
  #include <rfl.hpp>
  #include <rfl/json.hpp>
  
  struct Monster {
      rfl::Field<"name", std::string> name;
      rfl::Field<"hp", int> hp;
  };
  
  Monster m{.name = "Goblin", .hp = 50};
  std::string json_str = rfl::json::write(m); 
  // 输出: {"name":"Goblin","hp":50}
  ```

支持
```
Type
Property
Method
Enum
Attributes
Templates
Overloads
Compile-time iteration
```
refl-cpp 官方也明确强调，它的元数据可以通过 `constexpr` 和模板元编程处理，编译器可以把这些遍历和操作内联成接近手写代码的结果；它还专门优化了未实际参与反射的类型，减少元数据实例化和编译时间