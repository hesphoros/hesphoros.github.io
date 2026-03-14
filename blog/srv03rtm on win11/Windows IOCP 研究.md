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
```