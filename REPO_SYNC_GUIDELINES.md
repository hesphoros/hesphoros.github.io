# 仓库同步规范

本文档用于约束哪些文件应提交到远端仓库，哪些文件仅保留在本地，避免生成产物和本地配置污染 Git 历史。

## 一、同步原则

1. **源码优先**：只同步可复现项目状态的源码、配置和文档。
2. **生成物不入库**：构建产物、脚本生成文件、临时缓存默认不提交。
3. **本地配置最小化**：编辑器/个人工作流配置尽量不提交，除非团队明确需要。
4. **历史清理谨慎执行**：一旦需要重写历史，必须使用 `--force-with-lease` 推送并通知协作者。

## 二、本仓库约定（当前）

### 1) 必须忽略/不提交

- `generate.py` 生成的 JSON：
  - `public/desktop_*.json`
  - `public/map.json`
  - `docs/desktop_*.json`
  - `docs/map.json`
- Obsidian 本地工作区配置：
  - `.obsidian/`
  - `blog/.obsidian/**`（插件目录除外）

### 2) 明确保留

- `public/musics.json`（业务数据，非 `generate.py` 自动生成）
- `blog/.obsidian/plugins/**`（按当前仓库策略保留插件内容）

## 三、日常提交流程

1. 先检查变更：

   ```bash
   git status --short
   ```

2. 只添加明确需要同步的文件：

   ```bash
   git add <file-or-dir>
   ```

3. 提交前复核 staged 内容：

   ```bash
   git diff --cached --name-status
   ```

## 四、生成产物清理

当本地出现大量 `generate.py` 产物时，使用仓库脚本：

```bash
python3 scripts/clean_generated_json.py --dry-run
python3 scripts/clean_generated_json.py --yes
```

- `--dry-run`：仅预览将删除的文件。
- `--yes`：跳过交互确认并实际删除。

## 五、历史清理（仅必要时）

当不应同步的文件已经进入历史，按以下流程：

1. 先确认 `.gitignore` 规则正确。
2. 使用 `git filter-repo` 清理历史。
3. 执行：

   ```bash
   git push --force-with-lease origin main
   ```

4. 在团队内通知：历史已重写，协作者需重新同步本地分支。

## 六、注意事项

- 不要把 `git add .` 当成默认操作。
- 如果新增脚本会产生新型生成文件，需同步更新 `.gitignore` 和本文档。
- 任何“看起来像缓存或中间产物”的文件，先确认是否可由源码重新生成，再决定是否入库。
