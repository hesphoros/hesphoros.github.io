## 1. 克隆项目到本地
```bash
git clone <repo-url>
cd <repo-name>
```

## 2. 创建并切换新分支（开发新功能/修复问题）
```bash
git checkout -b feature/xxx   # 建议用 feature/bugfix/hotfix/等前缀
```

## 3. 代码开发与提交
```bash
# 修改或新增代码
git diff                # 查看修改内容
git add .               # 添加所有修改到暂存区（可改为 git add <file>）
git commit -m "feat: 描述你的功能或修复"
```

## 4. 推送分支到远端
```bash
git push origin feature/xxx
```

## 5. 远端分支有更新时同步主干（main/master）
```bash
git checkout main
git pull origin main    # 或 master，视仓库主分支名而定
git checkout feature/xxx
git rebase main         # 或 git merge main
# 处理冲突（如有冲突，手动编辑冲突文件，add 后继续 rebase）
```

## 6. 推送同步后的分支（如遇 rebase 需强推）
```bash
git push -f origin feature/xxx    # rebase 后需强推，merge 则不需要 -f
```

## 7. 提交 Pull Request（PR）
- 在 GitHub 上发起 PR，选择合并目标为 main 或 develop（按团队规范）。
- 参与代码评审，修正意见。
- 项目维护者采用 **Squash and Merge** 合并，保证主分支历史整洁。

## 8. 合并后本地清理
```bash
git checkout main
git pull origin main
git branch -d feature/xxx      # 删除本地分支
git push origin --delete feature/xxx  # 删除远端分支（如不再需要）
```

## 9. 日常同步主干
```bash
git checkout main
git pull origin main
```

---

## ⚡️ 进阶建议

- **分支命名规范**：feature/xxx、bugfix/xxx、hotfix/xxx、release/xxx
- **提交信息规范**：[Conventional Commits](https://www.conventionalcommits.org/zh-hans/v1.0.0/)
- **建议开启 PR 审核、强制主分支保护**
- **自动化工具**：建议结合 CI（如 GitHub Actions）自动测试、Lint、格式化
- **常用命令备查**：
    - `git status` 查看状态
    - `git log --oneline --graph` 查看简洁历史
    - `git remote -v` 查看远端信息

## 参考资料

- [成功的 Git 分支管理模型](https://nvie.com/posts/a-successful-git-branching-model/)
- [Git 官方文档](https://git-scm.com/book/zh/v2)