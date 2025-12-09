# Git

# 对比远程仓库和本地仓库的差异

~~~bash
git fetch origin
git diff origin/main
~~~

![image-20250507162053266](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250507162053266.png)

![image-20250507162134478](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250507162134478.png)

# 查看当前提交

~~~bash
git show
~~~

这将展示当前分支最新一次提交的：

- 提交哈希（commit hash）
- 作者
- 日期
- 提交信息
- 变动的文件和代码差异（diff）

### ✅ 只看当前提交的哈希（ID）：

```bash
git rev-parse HEAD
```

输出例子：

```
e5f6a8d09c62aaf643c89f0a5f56a4ef9890c0ba
```

------

### ✅ 简要格式查看最近一次提交：

~~~bash
git log -1
~~~

### ✅ 查看当前 HEAD 的提交哈希

```bash
git rev-parse HEAD
```

只输出类似这样的 SHA：

```bash
a1b2c3d4e5f67890123456789abcdef12345678
```



# **Git日志及状态查看**

查看所有提交的日志记录，命令如下

~~~bash
git log
~~~

效果如下图所示：

![image-20250526134159306](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250526134159306.png)

当我们的日志越来越多，可能想要简化查看，可以输入如下命令

> –oneline：在一行显示简略信息

![image-20250526134318927](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250526134318927.png)

# Git 版本回退

~~~bash
git reset --hard 版本号  
~~~

版本号使用以下命令查看

~~~
git log --oneline
~~~

# Git 忽略文件

在 .git 文件夹同级目录下新增`.gitignore`的忽略文件并写入忽略规则

![image-20250526134633203](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250526134633203.png)

![image-20250526134751794](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250526134751794.png)
