# CreateFileW

```c++
HANDLE CreateFileW(
  [in]           LPCWSTR               lpFileName,
  [in]           DWORD                 dwDesiredAccess,
  [in]           DWORD                 dwShareMode,
  [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  [in]           DWORD                 dwCreationDisposition,
  [in]           DWORD                 dwFlagsAndAttributes,
  [in, optional] HANDLE                hTemplateFile
);
```

## lpFileName

要创建或打开的文件或设备的名称。您可以在此名称中使用正斜杠 (/) 或反斜杠 (\)。

要创建文件流，请指定文件名、冒号，然后是流的名称。有关详细信息，请参阅 [“文件流”](https://learn.microsoft.com/en-us/windows/desktop/FileIO/file-streams) 。

默认情况下，名称长度限制为 MAX_PATH 个字符。要将此限制扩展到 32,767 个字符，请在路径前添加“\\?\”。

## dwDesiredAccess

请求对文件或设备进行访问，可以概括为读取、写入、两者都可或两者都不可（零）。

最常用的值是 `GENERIC_READ` ， `GENERIC_WRITE` ，或两者兼有（ `GENERIC_READ | GENERIC_WRITE `）

如果此参数为零，则应用程序可以查询某些元数据，例如文件、目录或设备属性，而无需访问该文件或设备，即使 **GENERIC_READ** 访问权限已被拒绝

您不能请求与指定的共享模式相冲突的访问模式。 在已打开句柄的打开请求中设置 dwShareMode 参数。

## dwShareMode

请求的文件或设备共享模式，可以是读取、写入、读写双向、删除、以上全部或无（参见下表）。对属性或扩展属性的访问请求不受此标志的影响

如果此参数为零且 **CreateFile** 操作成功，则该文件或设备无法共享，并且在关闭该文件或设备的句柄之前也无法再次打开。有关详细信息，请参阅“备注”部分。

您不能请求与已打开句柄的现有请求中指定的访问模式冲突的共享模式。CreateFile 操作将失败，并且 GetLastError 函数将返回错误。 共享违规错误 。

要允许一个进程在另一个进程打开文件或设备时共享该文件或设备，请使用以下一个或多个值的兼容组合。

## lpSecurityAttributes

指向 [SECURITY_ATTRIBUTES 的](https://learn.microsoft.com/en-us/windows/win32/api/wtypesbase/ns-wtypesbase-security_attributes)指针 包含两个独立但相关的数据成员的结构：一个可选的安全描述符和一个布尔值 决定返回的句柄是否可以被子进程继承的值。

如果此参数为 **NULL** ，则返回句柄 **CreateFile** 不能被应用程序创建的任何子进程继承，并且与返回句柄关联的文件或设备会获得默认的安全描述符。

**CreateFile** 忽略了 打开现有文件或设备时会使用 **lpSecurityDescriptor** 成员，但会继续使用 **bInheritHandle** 成员。

结构体的 **bInheritHandle** 成员指定返回的句柄是否可以被继承。

## dwCreationDisposition

对存在或不存在的文件或设备执行的操作。

对于文件以外的设备，此参数通常设置为 **OPEN_EXISTING** 。

## dwFlagsAndAttributes

文件或设备属性和标志，其中 **FILE_ATTRIBUTE_NORMAL** 是文件最常见的默认值

此参数可以包含可用文件属性（ **FILE_ATTRIBUTE_\*** ）的任意组合。所有其他文件属性都会覆盖此参数。 **FILE_ATTRIBUTE_NORMAL** 

此参数还可以包含标志的组合（ **FILE_FLAG_** *用于控制文件或设备缓存行为、访问模式和其他特殊用途标志。这些标志可与任何 **FILE_ATTRIBUTE_** 值结合使用* 

此参数还可以通过指定来包含安全服务质量 (SQOS) 信息。 **SECURITY_SQOS_PRESENT** 标志。其他与 SQOS 相关的标志信息请参见属性表和标志表之后的表格

## hTemplateFile

具有 **GENERIC_READ** 访问权限的有效模板文件句柄。该模板文件提供待创建文件的文件属性和扩展属性

此参数可以为 **NULL** 。

打开现有文件时， **CreateFile** 会忽略此参数

# return value

如果函数成功，则返回值是指向指定文件、设备、命名管道或邮件槽的打开句柄。

如果函数执行失败，则返回值为 INVALID_HANDLE_VALUE 。要获取更详细的错误信息，请调用 GetLastError 函数 。