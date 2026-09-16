see: [Build Guide](https://github.com/apache/brpc/blob/master/docs/cn/getting_started.md)
【金山文档】 bRPC研读
https://p.kdocs.cn/s/RTXKCBBIADAB6


### 使用 CMake 编译 Debug 版本

以下命令适用于 Apple Silicon（M1/M2/M3/M4），使用 Homebrew 安装依赖，并生成
`compile_commands.json` 供 VSCode、clangd 等工具使用：

```shell
# 安装构建工具和 brpc 依赖
brew install cmake ninja openssl@3 protobuf gflags leveldb

# 在 brpc 根目录执行；使用独立构建目录，不会污染源码树
cmake -S . -B build-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DDEBUG=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk \
  -DCMAKE_PREFIX_PATH=/opt/homebrew \
  -DOPENSSL_ROOT_DIR=/opt/homebrew/opt/openssl@3

cmake --build build-debug --parallel
```

`CMAKE_BUILD_TYPE=Debug` 会启用 CMake 的调试符号；brpc 还使用独立的 `DEBUG`
选项控制是否定义 `NDEBUG`，因此 Debug 构建需要同时设置 `-DDEBUG=ON`。目前项目会为
所有构建类型统一追加 `-O2`，所以上述配置带有调试符号和断言，但不是 `-O0` 构建。

构建完成后：

* 编译数据库位于 `build-debug/compile_commands.json`；
* 库文件位于 `build-debug/output/lib/`，工具位于 `build-debug/output/bin/`；
* 可用 `file build-debug/output/bin/rpc_view` 检查产物是否为 `arm64`。

如果 CMake 报错 `tapi error: malformed file` 并指出
`/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk`，通常是旧版 Command Line
Tools SDK 与当前 Xcode 不匹配。安装并选择完整 Xcode 后，保留上面命令中的
`CMAKE_OSX_SYSROOT`；也可先执行：

```shell
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
```

若依赖安装在非默认 Homebrew 前缀（例如 Intel Mac 的 `/usr/local`），请相应修改
`CMAKE_PREFIX_PATH`、`OPENSSL_ROOT_DIR` 和相关头文件/库路径。
