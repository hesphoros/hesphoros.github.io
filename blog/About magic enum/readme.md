
### 🔹 安装 / 配置方式

因为是 **单头文件库**，非常轻量，有几种方案：

#### 1. **直接下载头文件**

* 去 [release](https://github.com/Neargye/magic_enum/releases) 下一个 `magic_enum.hpp`
* 放进你的项目 `include/` 目录
* CMake 里加：

  ```cmake
  include_directories(${CMAKE_SOURCE_DIR}/include)
  ```

#### 2. **CMake FetchContent**

推荐这种方式，自动拉取，不依赖 vcpkg/conan：

```cmake
include(FetchContent)
FetchContent_Declare(
  magic_enum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
  GIT_TAG        v0.9.6  # 选一个稳定版本
)
FetchContent_MakeAvailable(magic_enum)

target_link_libraries(your_target PRIVATE magic_enum::magic_enum)
```

#### 3. **Conan / vcpkg**

* Conan：`conan install magic_enum/...`
* vcpkg：`vcpkg install magic-enum`

不过你说不想用 vcpkg，那直接走 **FetchContent** 或 **头文件拷贝** 就行。

### 🔹 使用示例

```cpp
#include <magic_enum.hpp>
#include <iostream>

enum class CompressionAlgorithm { NONE, GZIP, ZSTD, LZ4, BROTLI, LZMA };

int main() {
    CompressionAlgorithm algo = CompressionAlgorithm::ZSTD;

    // enum -> string
    std::string name = std::string(magic_enum::enum_name(algo));
    std::cout << "Enum to string: " << name << std::endl;

    // string -> enum (optional)
    auto value = magic_enum::enum_cast<CompressionAlgorithm>("LZ4");
    if (value.has_value()) {
        std::cout << "Parsed: " << magic_enum::enum_name(value.value()) << std::endl;
    }

    // 遍历所有枚举
    for (auto v : magic_enum::enum_values<CompressionAlgorithm>()) {
        std::cout << "Available: " << magic_enum::enum_name(v) << std::endl;
    }

    return 0;
}
```

输出大概是：

```
Enum to string: ZSTD
Parsed: LZ4
Available: NONE
Available: GZIP
Available: ZSTD
Available: LZ4
Available: BROTLI
Available: LZMA
```

---

