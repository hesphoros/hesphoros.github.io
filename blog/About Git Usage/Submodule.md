在 Git 中，当你克隆了一个包含子模块（Submodule）的项目，或者需要为一个现有的项目引入/初始化子模块时，通常会遇到两种场景。下面为你整理了最常用的命令和最佳实践。

### 场景一：刚刚 Clone 了一个新项目，子模块目录是空的

当你刚从 GitHub/GitLab 把项目克隆下来时，子模块的文件夹默认是存在的，但**里面没有任何文件**。你需要运行以下组合命令来初始化并拉取它们：

#### 1. 一键初始化并更新（最常用）

```bash
git submodule update --init --recursive
```

- **`--init`**：初始化本地配置文件中未初始化的子模块。
    
- **`--recursive`**：如果子模块内部还嵌套了更深层次的“子子模块”，这个参数会递归地把它们全部初始化并拉取下来。
    

#### 2. 分步执行（原理拆解）

如果你想知道它底层做了什么，它实际上等同于以下两步：


```bash
# 第一步：注册子模块。将 .gitmodules 文件中的路径和 URL 登记到 .git/config 中
git submodule init

# 第二步：去远程仓库把子模块的代码真正 Clone/Fetch 过来并 checkout
git submodule update
```

### 场景二：一步到位，在 `git clone` 的同时初始化子模块

如果你在克隆主项目时就知道它有子模块，完全不需要等到克隆完了再跑去初始化。你可以在执行 `clone` 时加上 `--recurse-submodules` 参数：



```bash
git clone --recurse-submodules <主项目的Git仓库地址>
```

这样 Git 会在克隆完主项目后，**自动**帮你把所有的子模块（包括嵌套子模块）全部一条龙克隆并初始化好。

### 场景三：在一个已有项目中，新添加一个子模块

如果你是项目的架构师，想把别人的一个开源库（或者你自己的另一个独立仓库）作为子模块引入到当前项目中：


```bash
git submodule add <子模块仓库的URL> <存放在当前项目中的路径>
```

**例如：** 想把一个皮肤库加到你的项目的 `libs/HesTheme` 目录中：


```bash
git submodule add https://github.com/user/HesTheme.git libs/HesTheme
```

执行后，Git 会做三件事：

1. 在本地自动下载该仓库到 `libs/HesTheme`。
    
2. 生成或修改根目录下的 `.gitmodules` 配置文件。
    
3. 自动将这些改动加入到 Git 暂存区，你只需要 `git commit` 提交即可。
    

###  独家排坑小贴士（Submodule 常用后续命令）

1. **子模块代码更新了，怎么拉取最新版？**
    
    如果团队其他人更新了子模块的指针，你拉取主项目后，需要运行：

    
```bash
git submodule update --remote
```
    
2. **批量在所有子模块中执行命令：**
    
    比如你想看所有子模块当前都在哪个分支/Commit 上：


```bash
git submodule foreach 'git status'
```