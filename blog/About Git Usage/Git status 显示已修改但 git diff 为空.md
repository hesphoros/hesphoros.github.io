# Git status 显示已修改但 git diff 为空

## 问题现象

在仓库中，某个文件看起来没有任何可见修改，但 Git 仍然把它标记为已修改：

```bash
git status --porcelain -- "blog/About c-cpp/template/Template Specialization.md"
```

输出类似：

```text
 M "blog/About c-cpp/template/Template Specialization.md"
```

但执行：

```bash
git diff -- "blog/About c-cpp/template/Template Specialization.md"
```

可能没有任何输出。

## 典型原因

这通常不是内容真的改了，而是 Git 索引中的文件状态缓存（stat cache）与工作区文件元数据（如时间戳）不同步。

简单说：

- Git 先根据文件元数据快速判断“可能有变化”；
- 但真正按内容比较时，内容哈希可能完全一致；
- 于是出现“status 说变了，diff 却空白”的现象。

## 本次排查过程（可复用）

### 1. 先看是否真的有文本差异

```bash
git diff -- "<目标文件>"
```

如果为空，继续。

### 2. 看行尾与属性

```bash
git ls-files --eol -- "<目标文件>"
git check-attr --all -- "<目标文件>"
```

用于排除 `.gitattributes`、CRLF/LF 归一化问题。

### 3. 对比索引与工作区的内容哈希

```bash
git ls-files -s -- "<目标文件>"
git hash-object -- "<目标文件>"
```

如果两者哈希一致，说明内容未变。

### 4. 用 porcelain v2 看状态细节

```bash
git status --porcelain=v2 -- "<目标文件>"
```

若显示索引与 HEAD 一致、但工作区标记修改，基本可判断是索引状态缓存问题。

## 修复方式

刷新索引状态缓存：

```bash
git update-index --refresh -- "<目标文件>"
```

然后复查：

```bash
git status --porcelain -- "<目标文件>"
```

如果输出为空，说明误报已清除。

## 扩展：多个文件都这样怎么办

如果有一批文件都出现同样问题，可以做全量刷新：

```bash
git update-index --refresh
```

注意：如果输出 `needs update`，表示这些文件需要刷新/重新检查，这是正常提示，不代表内容冲突。

## 不建议的做法

在这种“无内容差异”的场景下，不建议直接使用以下破坏性命令：

```bash
git reset --hard
git checkout -- <file>
```

因为它们会丢弃你可能真正存在的本地修改。

## 一句话结论

`git status` 显示已修改但 `git diff` 为空，常见根因是索引状态缓存与文件元数据不同步；执行 `git update-index --refresh` 通常即可恢复正常。
