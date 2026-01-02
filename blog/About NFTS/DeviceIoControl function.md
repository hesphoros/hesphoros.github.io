# DeviceIoControl

DeviceIoControl函数是Windows API中用于与**设备驱动**通信的重要函数。它通过发送**IRP（I/O Request Packet）**与驱动程序进行交互。DeviceIoControl函数的原型如下：

```c
/**
 * @brief Windows API DeviceIoControl 
 * @param hDevice
 * * A handle to the device on which the operation is to be performed.
 * * The device is typically a volume, directory, file, or stream.
 * * To retrieve a handle to a device, use the `CreateFile` function. 
 * @param dwIoControlCode
 * * The control code for the operation, this value identifies the specific 
 * * operation to be performed. 
 * * The control codes for this function are defined in the `WinIoCtl.h` header file.
 * @param lpInBuffer
 * * A pointer to the input buffer that contains the data required to perform
 * * If the operation does not require input data, this parameter can be `NULL`.
 * @param nInBufferSize
 * * The size, in bytes, of the input buffer pointed to by the `lpInBuffer` parameter.
 * @param lpOutBuffer
 * * A pointer to the output buffer that is to receive the data returned by the operation.
 * * If the operation does not return data, this parameter can be `NULL`.
 * @param nOutBufferSize
 * * The size, in bytes, of the output buffer pointed to by the `lpOutBuffer` parameter.
 */
BOOL DeviceIoControl(
  [in]                HANDLE       hDevice,
  [in]                DWORD        dwIoControlCode,
  [in, optional]      LPVOID       lpInBuffer,
  [in]                DWORD        nInBufferSize,
  [out, optional]     LPVOID       lpOutBuffer,
  [in]                DWORD        nOutBufferSize,
  [out, optional]     LPDWORD      lpBytesReturned,
  [in, out, optional] LPOVERLAPPED lpOverlapped
);

```



`[in] hDevice`

检索设备句柄, 使用CreateFile函数创建

[DeviceIoControl function （ioapiset.h） - Win32 apps |Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-deviceiocontrol)

# IRP

IRP是Windows驱动程序中用于处理I/O请求的数据结构。DeviceIoControl函数通过IRP与驱动程序通信。IRP的构造过程如下：

1. **创建IRP**：由I/O管理器创建。
2. **设置控制码**：通过DeviceIoControl函数的`dwIoControlCode`参数设置。
3. **设置输入缓冲区**：通过`lpInBuffer`和`nInBufferSize`参数设置。
4. **设置输出缓冲区**：通过`lpOutBuffer`和`nOutBufferSize`参数设置。

```c
#include <windows.h>

int main() {
    HANDLE hDevice = CreateFile("\\\\.\\PhysicalDrive0", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("无法打开磁盘设备\n");
        return 1;
    }

    DWORD dwBytesReturned;
    DISK_GEOMETRY diskGeometry;
    if (!DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &diskGeometry, sizeof(diskGeometry), &dwBytesReturned, NULL)) {
        printf("无法获取磁盘信息\n");
        CloseHandle(hDevice);
        return 1;
    }

    printf("柱面数: %llu\n", diskGeometry.Cylinders);
    printf("磁道数: %u\n", diskGeometry.TracksPerCylinder);
    printf("扇区数: %u\n", diskGeometry.SectorsPerTrack);
    printf("每扇区字节数: %u\n", diskGeometry.BytesPerSector);

    CloseHandle(hDevice);
    return 0;
}
```

# 控制码

控制码是DeviceIoControl函数中用于指定I/O控制操作的参数。控制码的结构如下：

| 位数  | 含义     |
| ----- | -------- |
| 31    | 保留位   |
| 30-16 | 设备类型 |
| 15-14 | 传输方式 |
| 13-12 | 访问类型 |
| 11-2  | 功能码   |
| 1-0   | 方法     |

控制码可以通过以下宏定义：

```c++
#define CTL_CODE(DeviceType, Function, Method, Access) \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method)
```