# 📋 方案 1：在 Windows 中格式化虚拟硬盘（详细步骤）

## 方法 A：使用 VirtualBox 自带工具（最简单）

### 步骤 1：准备 Linux Live CD
1. 下载一个小型 Linux Live CD ISO（推荐使用 SystemRescue 或 GParted Live）
   - **SystemRescue**: https://www.system-rescue.org/Download/
   - **GParted Live**: https://gparted.org/download.php

### 步骤 2：在 VirtualBox 中配置
1. 打开你的 HOS 虚拟机设置
2. 在"存储"选项卡中
3. 将第二 IDE 控制器的光盘改为 SystemRescue/GParted ISO（临时替换 HesphorosOS.iso）
4. 确保启动顺序为：光驱 → 硬盘

### 步骤 3：启动 Live CD 并格式化
1. 启动虚拟机，会进入 Linux Live 环境
2. 打开终端，执行以下命令：

```bash
# 查看硬盘
sudo fdisk -l
# 应该能看到 /dev/sda (2GB)

# 创建分区表
sudo fdisk /dev/sda
```

在 fdisk 中按以下顺序操作：
```
o    # 创建新的 DOS 分区表
n    # 新建分区
p    # 主分区
1    # 分区号 1
Enter # 默认起始扇区
Enter # 默认结束扇区（使用全部空间）
t    # 更改分区类型
c    # FAT32 (LBA) - 类型代码 0x0C
a    # 设置启动标志
w    # 写入并退出
```

3. 格式化为 FAT32：
```bash
sudo mkfs.vfat -F 32 /dev/sda1
```

4. 挂载并创建测试文件：
```bash
sudo mkdir /mnt/test
sudo mount /dev/sda1 /mnt/test

# 创建一些测试文件
echo "Hello from HesphorosOS!" | sudo tee /mnt/test/README.TXT
echo "FAT32 test file" | sudo tee /mnt/test/TEST.TXT
sudo mkdir /mnt/test/DOCS
echo "Documentation" | sudo tee /mnt/test/DOCS/INFO.TXT

sudo umount /mnt/test
```

5. 关闭虚拟机，将光驱改回 HesphorosOS.iso

---

## 方法 B：使用 Windows 工具（无需 Linux）

### 选项 1：使用 OSFMount（免费工具）

#### 步骤 1：下载并安装
1. 访问：https://www.osforensics.com/tools/mount-disk-images.html
2. 下载并安装 OSFMount（免费版即可）

#### 步骤 2：挂载 VDI 文件
1. 找到你的 VDI 文件位置（通常在 `C:\Users\你的用户名\VirtualBox VMs\HOS\HOS.vdi`）
2. 打开 OSFMount（以管理员身份运行）
3. 点击 "Mount new..." 按钮
4. 选择你的 `HOS.vdi` 文件
5. **重要**：选择 "Mount as Read/Write"（读写模式）
6. 点击 OK，等待挂载完成
7. 系统会分配一个驱动器盘符（如 E:）

#### 步骤 3：初始化和格式化
1. 打开"此电脑/我的电脑"
2. 你应该能看到新的磁盘（但显示为"未初始化"）
3. 右键点击任务栏上的"此电脑" → 管理 → 磁盘管理
4. 找到新挂载的磁盘（2GB，未分配）
5. 右键点击 → 初始化磁盘 → 选择 MBR（主启动记录）
6. 右键点击未分配空间 → 新建简单卷
7. 按向导操作：
   - 卷大小：使用默认（全部空间）
   - 分配驱动器号：选择一个（如 E:）
   - 格式化此卷：
     * 文件系统：**FAT32**
     * 分配单元大小：默认
     * 卷标：HOS_DISK
     * 快速格式化：勾选
8. 完成

#### 步骤 4：添加测试文件
1. 打开格式化好的驱动器（如 E:）
2. 创建一些测试文件：
   - 新建文本文档 `README.TXT`，写入：`Hello from HesphorosOS!`
   - 新建文本文档 `CONFIG.SYS`，写入：`DEVICE=TEST.SYS`
   - 创建文件夹 `DOCS`
   - 在 DOCS 中创建 `INFO.TXT`

#### 步骤 5：卸载磁盘
1. 回到 OSFMount
2. 选择挂载的磁盘
3. 点击 "Dismount" 按钮
4. 确认卸载

---

### 选项 2：使用 VBoxManage（VirtualBox 命令行工具）

这是一个自动化脚本方法：

#### 步骤 1：创建 PowerShell 脚本
创建一个文件 `format_vdi.ps1`：

```powershell
# 设置路径（根据你的实际路径修改）
$vdiPath = "C:\Users\你的用户名\VirtualBox VMs\HOS\HOS.vdi"
$rawPath = "C:\Users\你的用户名\VirtualBox VMs\HOS\HOS.img"

# VirtualBox 安装路径
$vboxManage = "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe"

Write-Host "Converting VDI to RAW..." -ForegroundColor Cyan
& $vboxManage clonehd $vdiPath $rawPath --format RAW

Write-Host "Mounting RAW image..." -ForegroundColor Cyan
$mountResult = Mount-DiskImage -ImagePath $rawPath -StorageType RAW -Access ReadWrite -PassThru

Start-Sleep -Seconds 2

# 获取磁盘号
$disk = Get-DiskImage -ImagePath $rawPath | Get-Disk

Write-Host "Initializing disk..." -ForegroundColor Cyan
Initialize-Disk -Number $disk.Number -PartitionStyle MBR -Confirm:$false

Write-Host "Creating partition..." -ForegroundColor Cyan
$partition = New-Partition -DiskNumber $disk.Number -UseMaximumSize -IsActiv