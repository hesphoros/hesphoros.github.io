**此文试图从泄漏的Windows 2003 server源码中研究IOCP机制**

相关源码见：
```
srv03rtm\base\ntos\io
```

相关引用资料：
```
 Windows internal 4th edition
	Chapter 7 — I/O System
 Windows Driver Kit （WKD 3790）
	 I/O Manager
	 IRP Handing
	 Completion Routines
	 Driver Dispatch
 Inside the Windows NT I/O System 论文
 OSR Online （https://www.osronline.com/）
 Programming the Microsoft Windows Driver Model
 Scalable Servers Using IO Completion Ports
	 中文版 https://blog.csdn.net/han_jiang_xue/article/details/18270155	
	 
```

相关缩写翻译
```
IRP
	I/O Request Packets
CSC
	Client Side Caching
```

# Windows I/O 系统组件

Windows的io设计目标是： 为应用程序提供一个有关设备的抽象（包括硬件设备和软件设备）此设备具备以下抽象

- 统一的 跨设备的安全性和命名机制 以保护可共享资源
- 高性能 异步 基于数据包的I/O 已支持那些可伸缩的应用程序实现
- 动态加载和动态卸载
- PnP 支持即插即用
- ...

Windows I/O 系统包含了以下几个执行体组件以及驱动设备

- I/O 管理器是I/O系统的核心
  - 连接了“应用程序和系统组件“与”虚拟的 逻辑的和物理的设备“
  - 定义了基础设施来支持设备驱动程序
- 设备驱动程序
  - 通常为某一种特定设备提供IO接口
  - 其接受io管理器传送给他们的命令，当命令完成时他们通知io管理器
  - 其通常使用io管理器向想其他的共享设备或控制的设备驱动程序转送io命令
- PnP管理器与IO管理器和一种称为总线型驱动程序（bus driver）的设备驱动紧密的一起工作
  - 指导硬件资源的分配
  - 检测并响应硬件设备的加入和移除
    - 当PnP管理器和总线型驱动程序负责在检测到一个设备时，将该设备的驱动程序加载进来。
    - 当一个设备被加入到一个缺乏正确的设备驱动程序的系统中时，执行体即插即用组件就会请求用户模式PnP管理器的设备安装服务
- 电池管理器也和IO管理器紧密工作（电源状态的转变
- WMI
- 注册表被当作了一个数据库
  - 保存了当前系统上的附载的基本设备硬件的描述信息，以及关于驱动程序初始化和配置的设置信息
  ![](../picture/Pasted%20image%2020260314200426.png)
- INF文件是驱动程序安装文件，INF文件是一个特定的硬件设备与作为该设备住控制驱动程序之间的纽带
- HAL（硬件抽象层 ) 通过提供一组API将不同平台之间的差异进行隐藏。 从而使驱动程序与处理器和中断控制器的特殊性相相互隔离开。

# I/O管理器

I/O管理器作为I/O系统的核心，其内部定义了一个非常整齐的框架，在此框架内部

- I/O请求都被递交给设备驱动程序进行处理。
- I/O系统是包驱动的（packet driven）
  - 绝大多数的I/O请求都是通过一个I/O请求包 （IRP ，I/O request packet）来表示的
  - 其定义见下文
  - 这些I/O请求包被从一个I/O系统组件传递到一个I/O系统组件

IRP其在Windows 2003 中的定义类似如下

在Windows底层中所有的i/o操作都会变成IRP例如

所有 I/O 操作都变成 IRP，例如：

| 操作            | IRP                   |
| --------------- | --------------------- |
| ReadFile        | IRP_MJ_READ           |
| WriteFile       | IRP_MJ_WRITE          |
| CreateFile      | IRP_MJ_CREATE         |
| DeviceIoControl | IRP_MJ_DEVICE_CONTROL |

```c++
// I/O Request Packet (IRP) definition
typedef struct DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) _IRP {
    CSHORT Type;
    USHORT Size;
    PMDL MdlAddress;
    ULONG Flags;

    union {
        struct _IRP *MasterIrp;
        LONG IrpCount;
        PVOID SystemBuffer;
    } AssociatedIrp;

    LIST_ENTRY ThreadListEntry;
    IO_STATUS_BLOCK IoStatus;
    KPROCESSOR_MODE RequestorMode;
    BOOLEAN PendingReturned;
    CHAR StackCount;
    CHAR CurrentLocation;
    BOOLEAN Cancel;
    KIRQL CancelIrql;
    CCHAR ApcEnvironment;
    UCHAR AllocationFlags;
    PIO_STATUS_BLOCK UserIosb;
    PKEVENT UserEvent;
    union {
        struct {
            PIO_APC_ROUTINE UserApcRoutine;
            PVOID UserApcContext;
        } AsynchronousParameters;
        LARGE_INTEGER AllocationSize;
    } Overlay;

    PDRIVER_CANCEL CancelRoutine;
    PVOID UserBuffer;

    union {
        struct {
            union {
                KDEVICE_QUEUE_ENTRY DeviceQueueEntry;
                struct {
                    PVOID DriverContext[4];
                } ;
            } ;
            
            PETHREAD Thread;
            PCHAR AuxiliaryBuffer;

            struct {
                LIST_ENTRY ListEntry;
                union {
                    struct _IO_STACK_LOCATION *CurrentStackLocation;
                    ULONG PacketType;
                };
            };
            PFILE_OBJECT OriginalFileObject;
        } Overlay;

        KAPC Apc;
        PVOID CompletionKey;
    } Tail;

} IRP, *PIRP;

```

