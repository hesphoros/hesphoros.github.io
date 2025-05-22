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
