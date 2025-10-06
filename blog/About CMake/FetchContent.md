`FetchContent` 是 **CMake** 在 3.11+ 引入的一个模块，用来 **在 CMake 配置阶段自动拉取和集成外部依赖**。

它的作用类似 `git submodule` + `add_subdirectory`，但是更自动化。  
这样你不需要手动下载依赖、也不一定需要提前安装库，而是让 CMake 在构建时自动拉取。

---

### 🔹基本用法

```cmake
include(FetchContent)

# 声明一个依赖
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        11.0.2
)

# 下载并让它在当前项目中可用
FetchContent_MakeAvailable(fmt)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE fmt::fmt)
```

---

### 🔹常见场景

1. **替代 vcpkg / Conan 等包管理器**  
    如果你只想要几个小的依赖，可以直接用 `FetchContent` 自动拉取，不必配置完整的包管理器。
    
2. **保证依赖版本一致**  
    比如团队协作时，每个人的 `fmt` 都会被拉取到指定的 `GIT_TAG`，避免版本不一致问题。
    
3. **跨平台方便**  
    无需用户安装库，CMake 会自动下载并构建。
    

---

### 🔹优点

- 不需要手动安装依赖库。
    
- 项目更加自包含（clone 下来就能直接编译）。
    
- 和 `add_subdirectory` 一样，可以直接使用 target。
    

### 🔹缺点

- 如果依赖很多，CMake 配置时会下载很多东西，第一次构建会比较慢。
    
- 不如包管理器灵活（比如版本管理、系统级缓存）。
    

---

