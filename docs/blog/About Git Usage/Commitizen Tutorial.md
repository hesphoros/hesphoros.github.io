
## 1. 安装 Commitizen
github 地址: [Commitizen](https://github.com/commitizen/cz-cli)

在项目根目录运行：

```bash
npm install --save-dev commitizen
```

## 2. 选择并安装适配器（以 cz-conventional-changelog 为例）

```bash
npm install --save-dev cz-conventional-changelog
```

| 适配器名                                                     | 适用范围 | 规范                                                         | 适合人群/生态                               | 安装命令                             |
| ------------------------------------------------------------ | -------- | ------------------------------------------------------------ | ------------------------------------------- | ------------------------------------ |
| [cz-conventional-changelog](https://github.com/commitizen/cz-conventional-changelog) | 通用     | [Conventional Commits](https://www.conventionalcommits.org/) | JS/TS、Node、前端、后端、微服务等大部分项目 | `npm i -D cz-conventional-changelog` |
| [cz-customizable](https://github.com/leoforfree/cz-customizable) | 通用     | 可定制（基于 Conventional Commits 可自定义类型/问题）        | 需要自定义提交类型的项目                    | `npm i -D cz-customizable`           |
| [cz-jira-smart-commit](https://github.com/commitizen/cz-jira-smart-commit) | 通用     | 支持 JIRA issue 号，结合 JIRA 工作流                         | 使用 JIRA 的团队                            | `npm i -D cz-jira-smart-commit`      |
| [cz-git](https://github.com/Zhengqbbb/cz-git)                | 通用     | 支持 emoji、中文、脚手架交互                                 | 喜欢美观交互、emoji、中文提示               | `npm i -D cz-git`                    |

## 3. 配置 package.json

在 `package.json` 增加如下字段：

```json
"config": {
  "commitizen": {
    "path": "cz-conventional-changelog"
  }
}
```

## 4. 使用 Commitizen 提交

使用 Commitizen 代替 `git commit`，运行：

```bash
npx cz
# 或者
npm run commit
```



首次使用推荐加脚本到 package.json：

```json
"scripts": {
  "commit": "cz"
}
```

这样只需执行 `npm run commit` 即可交互式填写规范化 commit。

运行效果:

![image-20250528194312819](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250528194312819.png)



## 5. 配合 Husky 强制规范（可选）

- 安装 Husky 和 Commitlint：

  ```bash
  npm install --save-dev husky @commitlint/{config-conventional,cli}
  ```

- 新建 `commitlint.config.js`：

  ```js
  module.exports = { extends: ['@commitlint/config-conventional'] };
  ```

- 启用 git hook：

  ```bash
  npx husky install
  npx husky add .husky/commit-msg "npx --no-install commitlint --edit $1"
  ```

这样每次提交都会校验格式，不规范会被阻止。

---

## 6. 常见问题

- **如何自定义类型？**  
  可以用 [cz-customizable](https://github.com/leoforfree/cz-customizable) 适配器，自定义类型和交互流程。

- **支持哪些规范？**  
  cz-conventional-changelog 支持 Conventional Commits 规范，适用于大多数开源和企业团队。

---

## 7. 参考链接

- [Commitizen 官方文档](https://commitizen-tools.github.io/commitizen/)
- [cz-conventional-changelog](https://github.com/commitizen/cz-conventional-changelog)
- [Conventional Commits 规范](https://www.conventionalcommits.org/zh-hans/v1.0.0/)

---



**一键规范 commit，从 Commitizen 开始！**