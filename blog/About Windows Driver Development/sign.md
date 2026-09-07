既然你已经开启了 **Test Signing**，并且 `signtool.exe` 在：

```text
D:\Windows Kits\10\App Certification Kit\signtool.exe
```

下面给出一个完整的**开发测试驱动签名流程**，针对：

```text
D:\Download\QQDownload\HWIDSpoofer\Release\WindowsDriver.sys
```

---

## 1. 创建测试代码签名证书

打开 **管理员 PowerShell**：

```powershell
$cert = New-SelfSignedCertificate `
    -Type CodeSigningCert `
    -Subject "CN=Hesphoros WindowsDriver Test Certificate" `
    -KeyAlgorithm RSA `
    -KeyLength 2048 `
    -HashAlgorithm SHA256 `
    -CertStoreLocation "Cert:\LocalMachine\My" `
    -FriendlyName "Hesphoros WindowsDriver Test"
```

查看：

```powershell
$cert.Thumbprint
```
Thumbprint:
```
1752829B944E3DCD0B30E891312ED55CE77737F7
```

复制输出的 Thumbprint。

例如：

```text
43BB437D609866286DD839E1D00309F5
```

---

## 2. 导出测试证书

管理员 PowerShell：

```powershell
Export-Certificate `
    -Cert $cert `
    -FilePath "D:\Download\QQDownload\HWIDSpoofer\Release\HesphorosDriverTest.cer"
```

生成：

```text
Release
│
├── WindowsDriver.sys
└── HesphorosDriverTest.cer
```

---

## 3. 安装证书到系统信任

管理员 PowerShell：

### Trusted Root

```powershell
Import-Certificate `
    -FilePath "D:\Download\QQDownload\HWIDSpoofer\Release\HesphorosDriverTest.cer" `
    -CertStoreLocation "Cert:\LocalMachine\Root"
```

### Trusted Publishers

```powershell
Import-Certificate `
    -FilePath "D:\Download\QQDownload\HWIDSpoofer\Release\HesphorosDriverTest.cer" `
    -CertStoreLocation "Cert:\LocalMachine\TrustedPublisher"
```

---

## 4. 使用 signtool 签名驱动

你的 signtool：

```text
D:\Windows Kits\10\App Certification Kit\signtool.exe
```

管理员 CMD：

```cmd
.\signtool.exe" sign ^
 /v ^
 /fd SHA256 ^
 /sha1 1752829B944E3DCD0B30E891312ED55CE77737F7 ^
 /tr http://timestamp.digicert.com ^
 /td SHA256 ^
 "D:\Download\QQDownload\HWIDSpoofer\Release\WindowsDriver.sys"
```

替换：

```text
YOUR_THUMBPRINT
```

为第 1 步得到的 Thumbprint。

例如：

```cmd
/sha1 43BB437D609866286DD839E1D00309F5
```

---

## 5. 验证签名

执行：

```cmd
"D:\Windows Kits\10\App Certification Kit\signtool.exe" verify ^
 /kp ^
 /v ^
 "D:\Download\QQDownload\HWIDSpoofer\Release\WindowsDriver.sys"
```

成功应该类似：

```text
Successfully verified: WindowsDriver.sys
```

---

## 6. 确认 Test Mode

管理员 CMD：

```cmd
bcdedit /enum {current} | findstr /i testsigning
```

应该：

```text
testsigning        Yes
```

如果没有：

```cmd
bcdedit /set testsigning on
```

然后重启。

桌面右下角应该显示：

```text
测试模式
Windows 11
```

---

## 7. 重新注册驱动

如果之前创建失败，删除：

```cmd
sc.exe stop WindowsDriver
sc.exe delete WindowsDriver
```

重新创建：

```cmd
sc.exe create WindowsDriver ^
type= kernel ^
start= demand ^
binPath= "D:\Download\QQDownload\HWIDSpoofer\sign-version\WindowsDriver.sys"
```

加载：

```cmd
sc.exe start WindowsDriver
```

查看：

```cmd
sc.exe query WindowsDriver
```

---

## 8. 如果仍然 577

检查：

```cmd
"D:\Windows Kits\10\App Certification Kit\signtool.exe" verify /kp /v ^
"D:\Download\QQDownload\HWIDSpoofer\Release\WindowsDriver.sys"
```

以及：

```powershell
Get-WinEvent -LogName "Microsoft-Windows-CodeIntegrity/Operational" -MaxEvents 10
```

---

补充一点：你之前的：

```text
flightsigning Yes
```

不用管，它不是测试签名状态。

真正关键的是：

```text
testsigning Yes
```

以及：

```text
signtool verify /kp
```

这两个通过后，`sc start WindowsDriver` 才会进入驱动初始化阶段。