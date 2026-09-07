https://github.com/wesmar/KernelResearchKit/releases/tag/bypass-code-integrity

msinfo32


```
sc.exe create WindowsDriver type= kernel start= demand binPath= "D:\Download\QQDownload\HWIDSpoofer\Release\WindowsDriver.sys"

sc.exe start WindowsDriver

sc.exe query WindowsDriver

sc.exe stop WindowsDriver
sc.exe delete WindowsDriver
```


```
bcdedit /enum {current}
```