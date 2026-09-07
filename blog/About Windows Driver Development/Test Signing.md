

```text
                    本地驱动测试
                         │
             ┌───────────┴───────────┐
             ↓                       ↓
       准备开发环境              配置测试系统
       VS + WDK                  Secure Boot
             │                       │
             ↓                       ↓
          driver.sys             Test Mode
             │
             ↓
       创建测试证书
             │
             ↓
       安装测试证书
             │
             ↓
        SignTool 签名
             │
             ↓
       安装 Driver Package
             │
             ↓
       Start / Stop Driver
             │
             ↓
        WinDbg 调试
```

## 1. 安装 VS + WDK

你现在是 VS2026 环境的话，首先确保安装了对应的 **Windows Driver Kit (WDK)**。
[Windows SDK ](https://learn.microsoft.com/zh-cn/windows/apps/windows-sdk/)
[WDK Download](https://learn.microsoft.com/en-us/windows-hardware/drivers/other-wdk-downloads)
[The DWK](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
你最终应该能找到：

```text
signtool.exe
inf2cat.exe
```

通常在 Windows SDK/WDK 的：

```text
D:\Windows Kits\10\App Certification Kit
```



---

## 2. 创建测试代码签名证书

可以使用 PowerShell：

```powershell
$cert = New-SelfSignedCertificate `
    -Type CodeSigningCert `
    -Subject "CN=Hesphoros Driver Test" `
    -CertStoreLocation "Cert:\CurrentUser\hesphoros" `
    -HashAlgorithm SHA256 `
    -KeyExportPolicy Exportable
```

然后：

```powershell
$cert.Thumbprint
```

记下 Thumbprint。

---

## 3. 导出 PFX

例如：

```powershell
$pwd = ConvertTo-SecureString `
    "123456" `
    -AsPlainText `
    -Force

Export-PfxCertificate `
    -Cert $cert `
    -FilePath "$env:USERPROFILE\Desktop\HesphorosDriverTest.pfx" `
    -Password $pwd
```

同时可以导出 `.cer`：

```powershell
Export-Certificate `
    -Cert $cert `
    -FilePath "$env:USERPROFILE\Desktop\HesphorosDriverTest.cer"
```

于是你有：

```text
HesphorosDriverTest.pfx
HesphorosDriverTest.cer
```

其中：

```text
.pfx
 ↓
私钥 + 证书
 ↓
用于签名

.cer
 ↓
公钥证书
 ↓
用于让测试系统信任它
```

---

# 4. 把测试证书加入信任

这是很多人第一次做驱动测试时最容易漏掉的。

将：

```text
HesphorosDriverTest.cer
```

安装到测试机器的：

```text
Trusted Root Certification Authorities
```

以及：

```text
Trusted Publishers
```

可以直接双击 `.cer`：

```text
安装证书
    ↓
本地计算机
    ↓
将所有的证书都放入下列存储
    ↓
Trusted Root Certification Authorities
```

然后再安装到：

```text
Trusted Publishers
```

也可以用 PowerShell/证书管理 API 自动完成。

---

# 5. 给驱动签名

如果你的驱动：

```text
visor.sys
```

可以使用 WDK 的：

```text
signtool.exe
```

例如：

```bat
signtool sign ^
    /fd SHA256 ^
    /f "%USERPROFILE%\Desktop\HesphorosDriverTest.pfx" ^
    /p 123456 ^
    visor.sys
```

签完以后检查：

```bat
signtool verify /v /pa visor.sys
```

或者：

```bat
signtool verify /kp /v visor.sys
```

`/kp` 是按照 **Kernel-Mode Code Signing policy** 进行验证，对驱动测试尤其有意义。

---

# 6. 如果是 PnP/WDM/KMDF 驱动

如果你的目录是：

```text
visor\
├── visor.sys
├── visor.inf
└── visor.cat
```

不要只签：

```text
visor.sys
```

更正规的测试流程是：

```text
visor.sys
     │
     ↓
生成 catalog
     │
     ↓
visor.cat
     │
     ↓
签名 visor.cat
     │
     ↓
安装整个 Driver Package
```

可以使用：

```bat
inf2cat
```

生成 `.cat`。

然后：

```bat
signtool sign ...
    visor.cat
```

最终：

```text
visor.inf
visor.sys
visor.cat
```

一起作为 Driver Package 安装。

对于 PnP 驱动，这是比单独签 `.sys` 更正确的流程。

---

# 7. 开启 Test Signing

检查：

```bat
bcdedit /enum
```

找：

```text
testsigning
```

开启：

```bat
bcdedit /set testsigning on
```

然后重启：

```bat
shutdown /r /t 0
```

启动后：

```text
桌面右下角
Test Mode
Windows 11
```

通常就表示 Test Signing 已经生效。

微软的测试签名机制就是用于开发阶段加载 test-signed kernel driver。

---

# 8.Secure Boot

你之前执行：

```bat
bcdedit /set testsigning on
```

得到：

```text
该值受安全引导策略保护
```

这基本就是 **Secure Boot** 导致的。

检查：

```powershell
Confirm-SecureBootUEFI
```

如果：

```text
True
```

就是开启。

这种情况下，你的开发测试机器通常需要进入：

```text
UEFI / BIOS
    ↓
Secure Boot
    ↓
Disabled
```

然后进入 Windows：

```bat
bcdedit /set testsigning on
```

再重启。

---

# 9. HVCI / Memory Integrity

还要检查：

```text
Windows Security
    ↓
Device Security
    ↓
Core Isolation
    ↓
Memory Integrity
```

如果你是在做自己的 kernel driver 开发，尤其是比较底层的：

```text
WDM
KMDF
Hyper-V
Virtualization
visor
```

开发环境最好注意 HVCI 的影响。

因为 **HVCI/Memory Integrity 对驱动的签名和兼容性要求更严格**。

所以开发机可能是：

```text
Secure Boot       OFF
Test Signing      ON
HVCI              根据驱动需求配置
```

而你的日常主力系统则不建议为了测试驱动长期降低这些安全措施。

---

# 10. 安装驱动

如果是 PnP Driver Package：

```bat
pnputil /add-driver visor.inf /install
```

查看：

```bat
pnputil /enum-drivers
```

---

如果是非 PnP kernel service，则可以使用 SCM：

```bat
sc create visor type= kernel binPath= "C:\Drivers\visor.sys"
```

启动：

```bat
sc start visor
```

停止：

```bat
sc stop visor
```

删除：

```bat
sc delete visor
```

注意：

```text
sc stop
```

和：

```text
sc delete
```

是两个不同操作。

例如：

```text
运行中的 visor
      │
      ├── sc stop visor
      │        ↓
      │      停止
      │
      └── sc delete visor
               ↓
           删除服务注册
```

如果你的驱动不支持卸载，那么 `sc stop` 可能失败，这取决于驱动本身的 `DriverUnload` 等实现。

---

# 11. 最后验证

检查签名：

```bat
signtool verify /kp /v visor.sys
```

查看驱动：

```bat
sc query visor
```

查看已安装驱动包：

```bat
pnputil /enum-drivers
```

还可以使用：

```text
WinDbg
```

连接到本机 kernel debugger。

---

# 你现在最适合的完整流程

如果你正在开发你前面提到的 `visor.sys`，我建议你的开发机配置成：

```text
Windows 11
   │
   ├── Visual Studio 2026
   ├── WDK
   ├── WinDbg
   │
   ├── Secure Boot = OFF
   ├── Test Signing = ON
   │
   └── Test Certificate
          │
          ├── .pfx
          │     └── 私钥
          │
          └── .cer
                └── Trusted Root / Trusted Publishers
                         │
                         ↓
                     SignTool
                         │
                         ↓
                    visor.sys
                         │
                         ↓
                  pnputil / sc.exe
                         │
                         ↓
                  Load / Stop / Unload
                         │
                         ↓
                       WinDbg
```

**核心是：你不需要购买证书。**

本地开发测试：

```text
自签名 Test Certificate
        +
SignTool
        +
Test Signing
        +
测试机信任证书
```

就可以完成。
