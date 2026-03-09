# macOS 从源码安装 libiconv：从踩坑到成功（Git 源码树 & release tar.gz）

本文记录一次在 macOS 上从源码安装 GNU **libiconv** 的完整过程：包括 **release tar.gz** 和 **GitHub 源码树（git clone）** 两种来源的区别、常见报错（`aclocal-1.xx`、`groff` 缺失、`/usr/local` 权限问题）以及最终推荐的安装方式（安装到自定义目录）。

> 适用对象：需要在 macOS 上自行编译 libiconv（静态/动态库均可），并希望安装到指定目录（如项目内目录、`~/.local`、`/opt/...`），而不是依赖 Homebrew 预编译包的人。

---

## 1. release tar.gz vs GitHub 源码树：到底差在哪？

### 1.1 直接下载 release tar.gz（面向用户构建）
**release tarball** 通常已经包含 autotools 生成的构建产物，例如：

- `configure`
- `Makefile.in`
- `config.h.in`

因此基本可以直接三连：

```sh
./configure --prefix=...
make
make install
```

一般 **不需要**你本机准备齐全的 autoconf/automake 版本组合，也更少遇到 `aclocal-1.xx` 之类的“工具链版本名”问题。

### 1.2 从 GitHub pull/clone 源码（面向开发者/维护者）
从 GitHub 拉下来的源码树通常只包含：

- `configure.ac`
- `Makefile.am`
- `m4/` 宏等

而 **不包含** `configure` 等生成物，所以你必须先运行：

- `./autogen.sh` 或
- `autoreconf -fi`

并依赖本机安装 autotools（autoconf/automake/libtool 等）。而一旦项目脚本写死了 `aclocal-1.16` / `aclocal-1.17` 这种带版本号的命令，你就会遇到本文后面讲的坑。

---

## 2. macOS 构建前准备（通用）

### 2.1 Xcode Command Line Tools
```sh
xcode-select --install
```

### 2.2（Git 源码树构建必备）安装 autotools
```sh
brew install autoconf automake libtool m4
```

验证：
```sh
which autoconf automake aclocal autoreconf
aclocal --version
```

---

## 3. 方式 A：从 release tar.gz 构建并安装（推荐优先尝试）

### 3.1 解压并确认有 configure
```sh
tar xf libiconv-*.tar.gz
cd libiconv-*
ls -la configure
```

如果没有 `configure`，大概率你拿到的不是官方 release tarball（可能是源码快照），建议更换下载源，或转到“方式 B”。

### 3.2 配置、编译、安装到自定义目录
推荐安装到用户可写目录，避免权限问题：

```sh
PREFIX="$HOME/.local/libiconv"

./configure --prefix="$PREFIX"
make -j"$(sysctl -n hw.ncpu)"
make install
```

### 3.3 验证安装结果
```sh
ls -l "$PREFIX/include/iconv.h"
ls -l "$PREFIX/lib" | egrep 'iconv|charset'
```

---

## 4. 方式 B：从 GitHub 源码树构建（踩坑集中地，但可控）

### 4.1 先生成 configure（autoreconf 或 autogen.sh）
在仓库根目录（包含 `configure.ac` 的目录）执行：

```sh
rm -rf autom4te.cache
autoreconf -fi
```

执行后应出现：
- `./configure`
- `build-aux/install-sh` 等辅助脚本

检查：
```sh
ls -la configure build-aux/install-sh
```

---

## 5. 常见坑 1：`aclocal-1.16` / `aclocal-1.17` command not found

### 5.1 现象
构建过程中报错类似：

- `/bin/sh: aclocal-1.17: command not found`
- `/bin/sh: aclocal-1.16: command not found`

这是因为项目脚本/Makefile **硬编码**了带版本号的命令名，但 Homebrew 安装的通常是无后缀的 `aclocal` / `automake`。

### 5.2 解决：在本地创建兼容软链接（推荐）
把 `aclocal-1.16` / `aclocal-1.17` 都指向 brew 的 `aclocal`：

```sh
mkdir -p "$HOME/.local/bin"

ln -sf "$(brew --prefix)/bin/aclocal"  "$HOME/.local/bin/aclocal-1.16"
ln -sf "$(brew --prefix)/bin/automake" "$HOME/.local/bin/automake-1.16"

ln -sf "$(brew --prefix)/bin/aclocal"  "$HOME/.local/bin/aclocal-1.17"
ln -sf "$(brew --prefix)/bin/automake" "$HOME/.local/bin/automake-1.17"

export PATH="$HOME/.local/bin:$PATH"
```

验证：
```sh
which aclocal-1.16
aclocal-1.16 --version
```

之后回到仓库根目录，重新跑：
```sh
autoreconf -fi
```

> 如果后续还报 `autoconf-2.71` / `autoheader-2.71` 缺失，可用同样方式创建软链接到 `autoconf` / `autoheader`。

---

## 6. 常见坑 2：`groff: command not found`（生成 man HTML 失败）

### 6.1 现象
编译接近完成时出现：
```text
/bin/sh: groff: command not found
make: *** [man/iconv.1.html] Error ...
```

这是文档生成依赖缺失，不影响核心库逻辑，但会让默认 `make` 目标失败。

### 6.2 解决：安装 groff（最省事）
```sh
brew install groff
```

然后继续：
```sh
make -j"$(sysctl -n hw.ncpu)"
make install
```

---

## 7. 常见坑 3：安装到 /usr/local 权限不足（Permission denied）

### 7.1 现象
```text
mkdir /usr/local/lib
mkdir: /usr/local/lib: Permission denied
...
make: *** [install] Error ...
```

### 7.2 解决思路
你有两条路：

**路 A（推荐）：不要装到 /usr/local，装到你有权限的目录**
例如 `~/.local/libiconv` 或项目目录。

**路 B：坚持 /usr/local，则用 sudo**
```sh
sudo make install
```

但更推荐路 A，避免把文件属主变成 root，后续升级/清理更麻烦。

---

## 8. 安装到“指定目录”的正确姿势（重点）

### 8.1 用绝对路径作为 prefix（最稳）
你想安装到源码目录下的 `build-install/`，用绝对路径：

```sh
PREFIX="$(pwd)/build-install"

make distclean 2>/dev/null || make clean

./configure --prefix="$PREFIX"
make -j"$(sysctl -n hw.ncpu)"
make install
```

> 不需要 sudo：因为你对当前目录有写权限。  
> 也不建议用 `sudo PREFIX=...` 这种写法，它不会影响后续命令，还可能把生成文件变成 root 属主。

### 8.2 安装后的目录结构
一般会得到：

- `$PREFIX/include/`（头文件）
- `$PREFIX/lib/`（库文件：`libiconv.*`、可能还有 `libcharset.*`）
- `$PREFIX/bin/`（工具：`iconv`）
- `$PREFIX/share/man/`（手册）

---

## 9. 你的工程如何引用自建 libiconv（简单示例）

假设安装到：
```sh
PREFIX="$HOME/.local/libiconv"
```

编译 C 程序时：
```sh
clang main.c -I"$PREFIX/include" -L"$PREFIX/lib" -liconv -o main
```

构建系统常用环境变量：
```sh
export CPPFLAGS="-I$PREFIX/include"
export LDFLAGS="-L$PREFIX/lib"
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH"
```

---

## 10. 结论与推荐路径

- **只想稳定安装**：优先用 **release tar.gz**（自带 `configure`，最少依赖、最少坑）。
- **需要最新提交/要改代码**：用 **GitHub 源码树**，但务必准备 autotools，并处理好 `aclocal-1.xx` 的兼容问题。
- **安装目录**：强烈建议使用 `--prefix` 指定到用户可写目录（如 `~/.local/...` 或项目内目录），避免 `/usr/local` 权限与 sudo 带来的后续维护麻烦。

---

## 附：一份“从 Git 源码树构建并安装到当前目录”的完整命令清单

```sh
# 0) 依赖
xcode-select --install
brew install autoconf automake libtool m4 groff

# 1) 兼容带版本号的 aclocal/automake（如果项目要求）
mkdir -p "$HOME/.local/bin"
ln -sf "$(brew --prefix)/bin/aclocal"  "$HOME/.local/bin/aclocal-1.16"
ln -sf "$(brew --prefix)/bin/automake" "$HOME/.local/bin/automake-1.16"
ln -sf "$(brew --prefix)/bin/aclocal"  "$HOME/.local/bin/aclocal-1.17"
ln -sf "$(brew --prefix)/bin/automake" "$HOME/.local/bin/automake-1.17"
export PATH="$HOME/.local/bin:$PATH"

# 2) 生成 configure
rm -rf autom4te.cache
autoreconf -fi

# 3) 配置/编译/安装到项目内目录
PREFIX="$(pwd)/build-install"
./configure --prefix="$PREFIX"
make -j"$(sysctl -n hw.ncpu)"
make install
```

注： 如果你觉得麻烦可以直接使用此跨平台编译方案
https://github.com/hesphoros/libiconv-native

