# Lazy Importer

github link: https://github.com/JustasMasiulis/lazy_importer

此文档用于学习Lazy Importer的作用机理，以及详细的底层逻辑。

此库的主要用途是为了隐藏 `Windows API / DLL导入信息` ，让程序更难被逆向分析。

```c
#include <Windows.h>

MessageBoxA(0,"hello","Title",0);
```

编译后，程序的IDT(导入表)里则会直接出现以下关键信息：

- MessageBoxA
- user32.dll

使用此库之后

- 不会在导入表中留下API的名称

- 运行时动态的解析函数地址

- 使用hash而不是字符串

- 在内存中都不出现明显的DLL/API字符串

  
# 为什么需要LazyImpoter
在标准的 Windows 开发中，当你调用一个 API（例如 `MessageBoxA` 或 `VirtualAlloc`）时，编译器通常会做两件事：

1. 在生成的二进制文件的 **导入地址表（IAT, Import Address Table）** 中留下这个函数的硬编码痕迹。
    
2. 在代码中留下明文的字符串（如 `"VirtualAlloc"`）用于动态加载。
    

反向工程师（或自动化沙箱）只需要查看 IAT 或者提取二进制文件中的明文参数字符串，就能轻易知道这个程序调用了哪些敏感的 API，从而快速推断出程序的功能。

**`lazy_importer` 的目的就是让这些特征彻底消失。**
# 核心工作原理
`lazy_importer` 通过以下几种技术组合，使得逆向分析变得极其困难：

- **动态哈希（Compile-time Hashing）：** 它不需要你传入 `"VirtualAlloc"` 这样的明文模块名或函数名字符串。相反，它在**编译期**利用 C++ `constexpr` 特性将函数名计算成一个哈希值。在运行期，它直接比对哈希值，从而避免了二进制文件中出现任何敏感的 API 字符串。
    
- **随机化哈希（Randomized Hashes）：** 为了防止逆向人员通过“已知哈希数据库”直接爆破反推函数名，`lazy_importer` 每次编译时都会引入随机的盐值（Salt），使得**每次编译生成的相同函数名的哈希值都完全不同**。
    
- **直接遍历 PE 结构：** 它不依赖 Windows 自带的 `GetProcAddress` 或 `LoadLibrary`（因为调用这两个 API 本身也会暴露痕迹）。它通过直接读取 `TEB/PEB`（进程环境块），在内存中手动遍历已加载模块的导出表（Export Table），并现场匹配哈希来定位函数地址。
    
- **高度内联（Force Inline）：** 其内部实现几乎全部使用了强行内联（`__forceinline`），这意味着在反汇编器（如 IDA Pro）中，你不会看到一个统一的 `get_proc_address_custom()` 函数，所有寻找 API 的代码都会被打碎并融进你的业务逻辑中，极大地增加了控制流的复杂性。