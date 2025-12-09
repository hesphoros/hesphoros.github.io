## 1. 分支策略

### 主要分支
- **main（或 master）**：始终保持可部署状态，仅合并经过充分测试的功能。
- **develop**：集成所有开发中的功能分支，定期同步到 main。

### 辅助分支
- **feature/\***：新功能开发，每个新功能一个分支，从 develop 分出，开发完成后合并回 develop。
- **bugfix/\***：针对 develop 上的 bug 修复，修复后合并回 develop。
- **hotfix/\***：生产环境紧急修复，从 main 分出，修复后合并回 main 和 develop。
- **release/\***：准备发布版本的分支，从 develop 分出，做最终测试和小范围修复后合并到 main 和 develop。

## 2. 工作流程

1. **克隆仓库**
   ```bash
   git clone <repo-url>
   cd  <repo>
   git checkout develop
   ```

2. **开发新功能**
如何写出一个优秀的commit [[如何写一份好的 Git Commit 信息]]
   ```bash
   git checkout -b feature/your-feature-name develop
   # 开发、提交、推送
   git add .
   git commit -m "feat: your feature description"
   git push origin feature/your-feature-name
   ```

3. **PR 流程**
   - 提交 Pull Request（PR）到 develop 分支。
   - 代码评审（Code Review）。
   - 自动化测试通过后合并。

4. **发布流程**
   - 从 develop 分出 release/x.y.z 分支，做预发布准备。
   - 修复 release 阶段发现的问题。
   - 合并 release 到 main 和 develop。
   - 打 tag，发布正式版本。

5. **紧急修复**
   - 从 main 分出 hotfix/xxx。
   - 修复后合并到 main 和 develop。
   - 打 tag，发布补丁版本。

## 3. 代码规范和工具
- 强制使用 Pull Request，禁止直接 push 到 main/develop。
- 配置代码格式化工具（如 Prettier、Black）。
- 配置自动化测试和 CI（如 GitHub Actions、GitLab CI）。
- 使用 CODEOWNERS/Reviewers 进行代码审核。

## 4. 其他建议
- 定期同步远程 develop/main，防止分支漂移。
- 合理使用 rebase 和 merge，保持提交历史清晰。
- 提交信息规范化（如 Conventional Commits）。
- 定期清理无用分支。

