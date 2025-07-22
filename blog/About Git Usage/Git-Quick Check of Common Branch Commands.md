
## 1. 查看当前所在分支

```bash
git branch
```
或者带高亮显示当前分支（前面有 `*` 号）：

```bash
git status
```

## 2. 创建新分支

```bash
git branch 分支名
```
例如：

```bash
git branch feature/login
```

或**直接切换并创建新分支**：

```bash
git checkout -b 分支名
```
例如：

```bash
git checkout -b feature/login
```

## 3. 删除本地分支

```bash
git branch -d 分支名    # 安全删除（分支已合并到主分支才可删）
git branch -D 分支名    # 强制删除（未合并分支也可删，慎用）
```
例如：

```bash
git branch -d feature/login
git branch -D feature/login
```

## 4. 删除远程分支

```bash
git push origin --delete 分支名
```
例如：

```bash
git push origin --delete feature/login
```

---

**温馨提示：**
- 删除分支前建议先切换到其他分支（如 `main`）。
- 分支名区分大小写。