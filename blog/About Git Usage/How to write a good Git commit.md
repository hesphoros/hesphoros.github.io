
## 1. 格式与结构

推荐采用如下结构（[Conventional Commits](https://www.conventionalcommits.org/) 规范）：

```
<type>(<scope>): <subject>

<body>

<footer>
```

### 示例
```
feat(login): 支持第三方登录

为用户增加了使用微信和QQ进行第三方登录的功能。
优化了登录流程，提升用户体验。

关闭: #123
```

---

## 2. 各部分含义

- **type**：本次提交的类型（必填）
  - feat: 新功能
  - fix: 修复 bug
  - docs: 文档变更
  - style: 代码格式（不影响功能，如空格、分号等）
  - refactor: 代码重构（不包括 bug 修复、功能新增）
  - test: 增加或修改测试
  - chore: 构建流程或辅助工具变动
- **scope**：影响的范围（可选），比如模块、文件名等
- **subject**：简要描述（不超过50个字符，首字母小写，结尾不加句号）
- **body**：详细描述（可选），说明动机、解决方案、对比等
- **footer**：关联事项（可选），如关闭 issue、兼容性说明等

---

## 3. 写作要点

- 以动词原形开头，如“add”、“fix”、“remove”
- 说明“做了什么”，“为什么做”，必要时“怎么做”
- 保持简明扼要，一次只做一件事
- 避免无意义的描述（如“update”、“fix bug”），要具体
- 推荐英文撰写，团队有要求可用中文

---

## 4. 不好的 Commit 示例

- “update”
- “fix bug”
- “修改样式”
- “优化”

---

## 5. 好的 Commit 示例

- “feat(user): 增加用户注册功能”
- “fix(api): 修复接口返回数据类型错误”
- “docs(readme): 完善安装步骤说明”
- “refactor(auth): 重构鉴权逻辑，提升安全性”

---

## 6. 推荐工具

- [commitizen](https://github.com/commitizen/cz-cli)：交互式生成标准化 commit
	- [[Commitizen使用教程]]
- [lint-staged](https://github.com/okonet/lint-staged)：提交前自动格式化和检测
- [commitlint](https://commitlint.js.org/)：强制 commit 格式

---

**写好 commit 信息，是每一位开发者的责任，也是项目高效协作的基础。**
## 7. 常见场景举例

- **功能开发**  
  `feat(profile): 新增用户资料编辑页面`
- **修复 bug**  
  `fix(register): 修复注册表单手机号校验错误`
- **代码重构**  
  `refactor(api): 拆分用户接口请求逻辑`
- **优化性能**  
  `perf(home): 优化首页渲染性能，减少接口请求`
- **测试代码**  
  `test(utils): 补充日期工具的单元测试`
- **文档更新**  
  `docs(readme): 更新安装与启动说明`
- **配置变更**  
  `chore: 升级 eslint 配置到最新版`
- **UI调整**  
  `style(button): 调整按钮圆角和阴影样式`

---
## 8. 团队协作建议

- **保持粒度适中**  
  一次 commit 只做一件明确的事，避免“大杂烩型”提交。
- **频繁提交**  
  及时提交阶段性成果，有助于回溯和协作，不要憋到很晚才一次性提交。
- **合理拆分**  
  功能开发时，拆分为多个 commit（如：页面结构、接口对接、样式优化分别提交）。
- **代码评审友好**  
  提交信息写得具体，能帮助代码审查者快速了解每次更改的目的和内容。

---

## 9. 常见问题与解答

- **Q: 英文还是中文？**  
  A: 跨国团队推荐英文，纯中文团队可用中文。关键是保持一致，避免夹杂。
- **Q: 一次 commit 改了很多文件怎么办？**  
  A: 尽量拆分。可以用 `git add -p` 分阶段提交。
- **Q: commit 信息写错了怎么办？**  
  A: 用 `git commit --amend` 修正最后一次 commit 信息，或用 `git rebase -i` 修改历史 commit（注意协作分支慎用）。
- **Q: 不小心把无关变更也提交了怎么办？**  
  A: 用 `git reset`、`git checkout` 撤回无关变更，重新提交。

---

## 10. 参考命令及工具

- **分阶段提交**  
  ```bash
  git add -p
  ```
- **修正最后一次提交信息**  
  ```bash
  git commit --amend
  ```
- **交互式修改历史提交**  
  ```bash
  git rebase -i HEAD~3   # 修改最近3次提交
  ```
- **规范化工具集成**  
  在项目中集成 `commitlint`、`husky` 实现自动校验 commit 信息规范。

---

## 11. 推荐阅读

- [Conventional Commits 中文文档](https://www.conventionalcommits.org/zh-hans/v1.0.0/)
- [为什么要写好的 commit message?](https://www.ruanyifeng.com/blog/2016/01/commit_message_change_log.html)
- [How to Write a Git Commit Message](https://cbea.ms/git-commit/)

---

**一句话总结：每一条 commit 信息，既是对同事的交代，也是对未来自己的交代。**