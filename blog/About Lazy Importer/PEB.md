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

## RtlGetCurrentPeb
在 NTDLL 5.1 及更高版本中，这项简单的工作以导出函数的形式更简洁地实现
```c
//
// PEB
//
typedef struct _PEB PEB,*PPEB;
NTSYSAPI PPEB NTAPI RtlGetCurrentPeb(VOID){
	return NtCurrentTeb () -> ProcessEnvironmentBlock;
}

//对于其自身的底层用户模式编程，微软长期以来一直使用一个名为 NtCurrentPeb 的宏或内联例程，
PEB *NtCurrentPeb (VOID)
{
    return (PEB *) __readfsdword (FIELD_OFFSET (TEB, ProcessEnvironmentBlock));
}
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

typedef struct _PEB_LDR_DATA
{
    unsigned long                             Length;
    unsigned char                        Initialized;
    void*                                   SsHandle;
    LIST_ENTRY                 InLoadOrderModuleList;
    LIST_ENTRY               InMemoryOrderModuleList;
    LIST_ENTRY       InInitializationOrderModuleList;
    void*                            EntryInProgress;
    unsigned char                 ShutdownInProgress;
    void*                           ShutdownThreadId;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

PPEB_LDR_DATA ldr;
```

以下三个双向列表是PEB中最著名的部分,记录了当前进程加载的所有dll模块
```c
PEB
 └─Ldr
     ├─InLoadOrderModuleList  // 按模块加载的先后顺序排列
     ├─InMemoryOrderModuleList  //按模块加载在内存中的地址顺序排列
     └─InInitializationOrderModuleList  // 按照初始化的顺序排列
```

-  InLoadOrderModuleList


很多shellcode通过这里获取 kernel32.dll ntdll.dll user32.dll ,而不用调用系统API

# BeingDebugged
这是仅仅占用1字节的标志位(Boolean)
- 如果当前进程正在被调试器(如x64dbg,IDA pro,VS)附加.这个值就会被系统设为1
- 如果正常状态下则是0
在Windows API中的`IsDebuggerPresent()`函数,其底层实现及其简单,就是去读取PEB字段中的这个`BeingDebugged`字段,许多软件的反调试技术(Anti-Debugging)会通过直接检查该字段或手动将其清理,来检测或过滤掉调试器
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

# ProcessHeap/NumberOfHeaps 
进程堆信息
指向进程的默认堆指针,调试状态下,堆的创建行为和普通状态不同(会带有调试标记),这也是反调试技术经常用到的地方.
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

# ImageBaseAddress
映像基地址,记录了当前主程序(.exe)在内存中的起始虚拟地址


# PACTIVATION_CONTEXT_NOTIFY_ROUTINE
```c
/**
 * @brief Activation Context 通知回调函数。
 *
 * 当激活上下文（Activation Context）的状态发生变化时，
 * Windows Fusion/SxS 子系统会调用此回调函数进行通知。
 *
 * 典型通知包括：
 * - Activation Context 创建
 * - Activation Context 激活
 * - Activation Context 停用
 * - Activation Context 引用计数变化
 * - Activation Context 销毁
 *
 * @param NotificationType
 *        通知类型，指定当前发生的事件类型。
 *
 * @param ActivationContext
 *        触发通知的 Activation Context 对象。
 *
 * @param NotificationContext
 *        与通知相关的上下文信息。
 *
 * @param NotificationData
 *        附加通知数据，具体内容取决于 NotificationType。
 *
 * @param Context
 *        用户自定义上下文指针，通常在注册回调时传入。
 *
 * @note
 * 该回调属于 Windows 内部 Activation Context (SxS/Fusion) 机制，
 * 并非公开稳定 API，其参数和行为可能随 Windows 版本发生变化。
 *
 * @see ACTIVATION_CONTEXT
 * @see ACTIVATION_CONTEXT_DATA
 * @see ACTIVATION_CONTEXT_STACK
 */
typedef VOID (NTAPI* PACTIVATION_CONTEXT_NOTIFY_ROUTINE)(
    ULONG NotificationType,
    PACTIVATION_CONTEXT ActivationContext,
    PACTIVATION_CONTEXT_NOTIFICATION_CONTEXT NotificationContext,
    PVOID NotificationData,
    PVOID Context
);
```