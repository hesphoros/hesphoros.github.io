# 环境配置

`spdlog` 的配置方式取决于你想要的日志风格、输出位置和性能优化需求。下面我给你一个 **CMake + spdlog 基础配置** + **常用使用方式** 的总结：

------

## 1. 获取 spdlog

你不想用 vcpkg，可以选择以下方式：

### 方式 A：直接添加子模块

```bash
git submodule add https://github.com/gabime/spdlog.git extern/spdlog
```

然后在 `CMakeLists.txt` 里：

```cmake
add_subdirectory(extern/spdlog)

target_link_libraries(your_project PRIVATE spdlog::spdlog)
```

### 方式 B：FetchContent（推荐）

```cmake
include(FetchContent)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.14.1 # 选你需要的版本
)
FetchContent_MakeAvailable(spdlog)

target_link_libraries(your_project PRIVATE spdlog::spdlog)
```

这样就不用 vcpkg，也不需要手动安装。

------

## 2. 基础配置（代码里）

### 控制日志级别

```cpp
#include <spdlog/spdlog.h>

int main() {
    spdlog::set_level(spdlog::level::debug); // 全局日志级别
    spdlog::info("Hello, {}!", "world");
    spdlog::debug("Debug message");
}
```

### 输出到控制台 + 文件

```cpp
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink    = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/mylog.txt", true);

    std::vector<spdlog::sink_ptr> sinks { console_sink, file_sink };
    auto logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());

    spdlog::register_logger(logger);

    logger->info("log to console and file");
}
```

------

## 3. 常用优化

- **异步日志**（高性能，适合服务器/文件传输场景）：

```cpp
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>

int main() {
    spdlog::init_thread_pool(8192, 1); // 队列大小 & 线程数
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("async_log.txt", true);

    auto async_logger = std::make_shared<spdlog::async_logger>(
        "async_file", file_sink, spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);

    spdlog::register_logger(async_logger);

    async_logger->info("This is async log!");
}
```

- **日志格式**（默认 `[时间] [logger名] [级别] 内容`）：

```cpp
spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
```

例子输出：

```
[2025-09-30 21:23:45.123] [main] [INFO] hello world
```


