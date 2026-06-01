# Core Principle

Lazy_Importer的核心原理：
1. 获取当前进程的PEB(Process Environment Block)
2. 遍历已加载模块
3. 解析PE(Portable Executable)导出表
4. 哈希匹配函数名
5. 获取函数地址

PEB
PE格式
IMAGE_EXPORT_DIRECTORY
LdrDataTableEntry
