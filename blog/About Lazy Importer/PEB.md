# PEB (**Process Environment Block**)

它是 Windows 内核为每个用户模式进程创建的一个非常核心的数据结构。PEB 存在于**用户态内存**空间中，包含了整个进程的全局配置信息、加载的模块列表、环境变量、堆信息以及进程的运行状态。

PEB其创建流程为
```c
NtCreateUserProcess->PspCreateProcess->MmCreatePeb
```
在 `MmCreatePeb()` 中分配并初始化：
```c
Peb->ImageBaseAddress
Peb->ProcessHeap
Peb->NumberOfProcessors
Peb->OSMajorVersion
Peb->OSMinorVersion
```
然后：
```c
NtCreateThread
    ↓
MmCreateTeb
```

由于它位于用户态，这意味着**进程自身的代码（包括应用层代码、反作弊系统、甚至是恶意软件）可以直接读取它**，而不需要频繁地通过系统调用（Syscall）切换到内核态，从而大大提高了系统的运行效率。

# PEB 在内存中的关键位置
## 32位进程下
```
mov eax, fs:[30h]
```
得到
```c
PPEB Peb = (PPEB)__readfsdword(0x30);
```
对应关系为:
```c
FS:[0x18] -> TEB
FS:[0x30] -> PEB
```

TEB 中：

```c
TEB->ProcessEnvironmentBlock
```
## win11 x64下
在64位进程中
```c
PPEB Peb = (PPEB)__readgsqword(0x60);
```

```
GS:[0x30] -> TEB
GS:[0x60] -> PEB
```

# PEB 的核心作用与内部关键信息

PEB 结构体非常庞大且大部分字段被微软隐藏（未公开或标记为保留字段），但通过逆向工程和文档公开，其最核心的几个成员包括：
- LoaderData (PPEB_LDR_DATA)
- BeingDebugged
- ImageBaseAddress
- ProcessHeap/NumberOfHeaps
- ProcessParameters


# PPEB_LDR_DATA
```c
PPEB_LDR_DATA ldr;
```

```c
PEB
 └─Ldr
     ├─InLoadOrderModuleList
     ├─InMemoryOrderModuleList
     └─InInitializationOrderModuleList
```

很多shellcode通过这里获取 kernel32.dll ntdll.dll user32.dll ,而不用调用系统API

# ProcessParameters
```c
RTL_USER_PROCESS_PARAMETERS
```

internal：
```c
CommandLine
ImagePathName
CurrentDirectory
Environment
```

例如： 这样就能获取命令行
```c
Peb->ProcessParameters->CommandLine
```

# ProcessHeap
```c
PVOID ProcessHeap;
```
其对应:
```c
GetProcessHeap();
```
实际上就是返回：
```c
NtCurrentPeb()->ProcessHeap
```