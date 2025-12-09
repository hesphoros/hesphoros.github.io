# 高性能文件上传客户端 - Qt项目

这是一个基于Qt6和CMake的高性能文件上传客户端UI项目。

## 项目结构

```
client/
├── CMakeLists.txt          # CMake构建配置
├── build_optimized.bat     # Windows一键构建脚本
├── README.md               # 项目说明
├── client.md               # 架构设计文档
├── examples/               # 使用示例
│   └── minimal_upload_example.cpp  # 极简调用示例
├── include/                # 头文件目录
│   ├── FileInfo/           # 文件信息结构
│   │   └── FileInfo.h      
│   ├── SyncUploadQueue/    # 极简上传队列（核心）
│   │   └── Lusp_SyncUploadQueue.h
│   ├── ThreadSafeRowLockQueue/  # 行级锁队列（核心）
│   │   └── ThreadSafeRowLockQueue.hpp
│   ├── MainWindow.h        # 主窗口类
│   ├── FileListWidget.h    # 文件列表控件
│   └── FileInfoWrapper.h   # 文件信息包装器
├── src/                    # 源文件目录
│   ├── main.cpp            # 程序入口
│   ├── MainWindow.cpp      # 主窗口实现
│   ├── FileListWidget.cpp  # 文件列表实现
│   ├── SyncUploadQueue/    # 极简上传队列实现
│   │   └── Lusp_SyncUploadQueue.cpp
│   └── FileInfoWrapper.cpp # 文件信息包装器实现
├── 3rdParty/               # 第三方依赖
│   └── include/md5.h       # MD5计算
└── ui/                     # UI设计文件
    └── MainWindow.ui       # Qt Designer UI文件
```

## 环境要求

### 必需软件
- **Qt6** (6.2或更高版本)
  - 包含 Qt6::Core 和 Qt6::Widgets 模块
- **CMake** (3.16或更高版本)
- **C++17兼容编译器**
  - Windows: Visual Studio 2019/2022
  - Linux: GCC 7+ 或 Clang 6+
  - macOS: Xcode 10+

### Qt6安装建议
- **Windows**: 从 [Qt官网](https://www.qt.io/download) 下载Qt Online Installer
- **Linux**: 使用包管理器安装 qt6-base-dev
- **macOS**: 使用Homebrew: `brew install qt6`

## 构建方法

### Windows (推荐)

1. **使用构建脚本**:
   ```bash
   cd client
   build.bat
   ```

2. **手动构建**:
   ```bash
   # 设置Qt6路径（如果需要）
   set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64
   
   # 创建构建目录
   mkdir build
   cd build
   
   # 配置项目
   cmake .. -G "Visual Studio 17 2022" -A x64
   
   # 构建项目
   cmake --build . --config Release
   ```

### Linux/macOS

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 构建项目
cmake --build . -j$(nproc)

# 运行程序
./bin/UploadClient
```

## 运行程序

构建成功后，可执行文件位于：
- Windows: `build/bin/Release/UploadClient.exe`
- Linux/macOS: `build/bin/UploadClient`

## 功能特性

### 当前已实现
- ✅ **极简分层架构**: 基于client.md设计的真实分层架构
- ✅ **行级锁队列**: ThreadSafeRowLockQueue，标准C++实现
- ✅ **UI极简调用**: 只需`Upload::push(filePath)`或`queue.push()`
- ✅ **通知线程独立**: 条件变量精确唤醒，专门处理队列
- ✅ **Qt6现代UI界面**: 支持拖拽、文件选择、状态显示
- ✅ **文件拖拽支持**: 拖拽即上传，UI立即响应
- ✅ **文件选择对话框**: 多文件选择，批量上传
- ✅ **文件列表管理**: 添加、显示、清空文件列表
- ✅ **进度条和状态显示**: 可选的进度回调和状态更新
- ✅ **菜单栏和快捷键**: 完整的UI交互体验
- ✅ **零延迟响应**: UI操作立即返回，用户体验极佳

### 架构设计特点
- **🎯 UI线程极简**: 只需`queue.push()`一行代码，立即返回
- **🧵 通知线程独立**: 专门的线程处理队列，条件变量精确唤醒
- **🔒 行级锁队列**: 入队和出队锁分离，支持高并发，UI不阻塞
- **📞 本地服务分离**: 通知线程只负责消息传递，真实上传由本地服务处理
- **⚡ 零延迟响应**: UI操作立即返回，用户感觉不到任何卡顿
- **🛠️ 标准C++实现**: 基于std::queue+std::mutex+std::condition_variable，不依赖Qt锁
- **🎨 模块化设计**: 清晰的职责分离和接口定义，易于维护和扩展

### 待实现功能
- 🔄 真实的网络上传逻辑
- 🔄 断点续传支持
- 🔄 多线程并发上传
- 🔄 上传速度限制
- 🔄 服务器连接配置
- 🔄 上传历史记录

## 故障排除

### 常见问题

1. **CMake找不到Qt6**
   ```bash
   # 设置Qt6安装路径
   set CMAKE_PREFIX_PATH=C:\path\to\qt6
   ```

2. **编译错误: 找不到Qt头文件**
   - 确保Qt6完整安装，包含开发文件
   - 检查CMAKE_PREFIX_PATH是否正确

3. **运行时错误: 找不到Qt6 DLL**
   - Windows: 将Qt6 bin目录添加到PATH
   - 或者使用windeployqt部署工具

4. **Linux编译错误**
   ```bash
   # Ubuntu/Debian
   sudo apt install qt6-base-dev qt6-base-private-dev
   
   # Fedora
   sudo dnf install qt6-qtbase-devel
   ```

### 调试模式

构建Debug版本用于开发调试：
```bash
cmake --build . --config Debug
```

## 开发说明

### 添加新功能
1. 在相应的头文件中声明接口
2. 在源文件中实现功能
3. 更新CMakeLists.txt（如有新文件）
4. 重新构建项目

### 代码规范
- 使用Qt命名约定（camelCase）
- 成员变量以m_前缀
- 信号槽连接使用新语法
- 优先使用智能指针管理内存

### 架构扩展
当前项目为UI层框架，可以轻松扩展：
- 替换UploadManager中的模拟逻辑为真实网络代码
- 添加更多文件类型支持
- 集成认证和权限管理
- 添加配置文件支持

## 🚀 极简使用示例

### UI开发者视角（极简到极点）

```cpp
#include "SyncUploadQueue/Lusp_SyncUploadQueue.h"

// 🎯 方式1：使用便捷接口（推荐）
void uploadFiles() {
    // UI线程的全部工作就是这些一行代码！
    Upload::push("C:/documents/report.pdf");           // 单文件上传
    Upload::push({"file1.txt", "file2.jpg", "video.mp4"}); // 批量上传
    
    // 完事！UI立即返回，用户感觉零延迟
    // 剩下的全部由通知线程和本地服务自动处理
}

// 🎯 方式2：带可选进度回调
void uploadWithProgress() {
    auto& queue = Lusp_SyncUploadQueue::instance();
    
    // 可选：设置进度回调
    queue.setProgressCallback([](const std::string& filePath, int percentage, const std::string& status) {
        std::cout << "📊 " << filePath << " - " << percentage << "%" << std::endl;
    });
    
    // 上传文件（立即返回）
    queue.push("C:/uploads/large_file.zip");
    
    // 查询状态（无锁原子操作）
    std::cout << "队列中文件数: " << queue.pendingCount() << std::endl;
}
```

### Qt GUI集成示例

```cpp
// 用户拖拽文件 - UI线程极简处理
void MainWindow::dropEvent(QDropEvent *event) {
    QStringList filePaths;
    for (const QUrl& url : event->mimeData()->urls()) {
        filePaths << url.toLocalFile();
    }
    
    // 🎯 UI线程的全部工作：把文件丢进队列就完事！
    addFilesToUploadQueue(filePaths);
    // 立即返回，UI继续响应用户操作
}

void MainWindow::addFilesToUploadQueue(const QStringList& filePaths) {
    std::vector<std::string> stdFilePaths;
    for (const QString& path : filePaths) {
        stdFilePaths.push_back(path.toStdString());
    }
    
    // 🎯 这就是全部！UI线程只需要这一行代码！
    Lusp_SyncUploadQueue::instance().push(stdFilePaths);
    
    m_statusLabel->setText("文件已提交上传队列");
}
```

### 架构工作流程

```
🎯 用户操作 → UI检测文件 → queue.push() → 立即返回 ✅
     ↓ (后台自动处理，UI无需关心)
🧵 通知线程 → 条件变量唤醒 → 给本地服务发通知
     ↓
🏢 本地服务 → 接收通知 → 后台上传 → 进度回调 → UI显示（可选）
```

**🎉 核心优势**：UI开发者只需要知道一个接口`Upload::push()`，剩下的全部透明处理！

## 许可证

本项目遵循MIT许可证。详见LICENSE文件。
