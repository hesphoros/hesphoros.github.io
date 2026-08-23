# Glaze

**特点**：追求极致性能的 C++20 静态反射与 JSON 序列化库，性能常常超越 SIMDJSON。

**原理**：利用 C++20 `constexpr` 字符串和编译期元组实现零开销映射。

**示例**：

```c
#include <glaze/glaze.hpp>

struct Player {
    std::string name;
    int score;
};

// 注册映射关系（编译期构建）
template <>
struct glz::meta<Player> {
    using T = Player;
    static constexpr auto value = object("name", &T::name, "score", &T::score);
};
```

