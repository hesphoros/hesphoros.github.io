# Magic_Enum

**特点**：专门针对 **Enum（枚举）** 的无侵入静态反射库，极度轻量，只需包含头文件。

**功能**：枚举转字符串、字符串转枚举、获取枚举元素个数、遍历枚举。

**实现原理**：利用编译期的 `__PRETTY_FUNCTION__` / `__FUNCSIG__` 提取枚举名的字符串。

**示例**：

```c++
#include <magic_enum.hpp>

enum class Color { RED, GREEN, BLUE };

// 1. 枚举转字符串
Color c = Color::RED;
auto name = magic_enum::enum_name(c); // "RED"

// 2. 字符串转枚举
auto color = magic_enum::enum_cast<Color>("GREEN"); // Color::GREEN
```