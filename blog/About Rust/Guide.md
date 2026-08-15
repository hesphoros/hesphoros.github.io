学习 Rust 的最佳路径通常结合了**官方文档**与 **GitHub 上高星开源项目/路线图**。以下是为你整理的 Rust 学习路线、官方权威文档以及 GitHub 上最优质的学习资源。

---

## 阶段学习路线

### 1. 入门夯实基础

* **核心目标**：理解 Rust 独有的所有权（Ownership）、借用（Borrowing）、生命周期（Lifetime）与类型系统。
* **重点概念**：`Option`/`Result` 错误处理、Trait 接口、模式匹配（Pattern Matching）。
* **推荐实操**：用 Rust 实现基础数据结构（如链表）、简单命令行工具。

### 2. 进阶高阶特性

* **核心目标**：掌握 Rust 进阶并发模型与底层编程技巧。
* **重点概念**：Smart Pointers (`Box`, `Rc`, `Arc`, `RefCell`)、多线程（Threads, `mpsc`）、Async/Await 异步编程、Unsafe Rust。
* **推荐实操**：编写一个多线程 Web 服务器、异步 HTTP 爬虫。

### 3. 项目实战与生态选型

根据应用方向选择对应的生态栈：

* **Web 后端 / API**：Axum, Actix-web, Tokio, Diesel/SQLx
* **命令行工具 (CLI)**：Clap, Anyhow, Indicatif
* **系统 / 嵌入式**：Cargo 交叉编译, Embedded-hal, Cross
* **WebAssembly (Wasm)**：Wasm-bindgen, Yew/Leptos
* **密码学 / 区块链**：Substrate, Solana

---

## 官方权威文档 (Official Docs)

Rust 的官方文档质量极高，建议作为第一参考源：

| 资源名称 | 说明 | 适用阶段 |
| --- | --- | --- |
| **[The Rust Programming Language](https://doc.rust-lang.org/book/)** <br>

<br>*(俗称 The Book)* | **必读**！Rust 最权威的入门教程，非常系统地解释了所有权等核心概念。 | 入门 |
| **[Rust by Example](https://doc.rust-lang.org/rust-by-example/)** | 通过大量带交互/可运行的代码示例来学习 Rust 语法。 | 入门 / 快速查阅 |
| **[Rust Standard Library (std)](https://doc.rust-lang.org/std/)** | 官方标准库 API 文档，搜索内置数据结构和函数的用法。 | 阶段贯穿 |
| **[The Cargo Book](https://doc.rust-lang.org/cargo/)** | Rust 包管理器和构建工具 Cargo 的指南，讲解依赖管理与工作空间。 | 基础 |
| **[The Async Book](https://rust-lang.github.io/async-book/)** | 官方异步编程指南，深入讲解 Async/Await 与 Futures。 | 进阶 |
| **[The Rustonomicon](https://doc.rust-lang.org/nomicon/)** | *(死灵书)* 深入解释 Unsafe Rust 和底层内存模型的极客指南。 | 高阶 |

> 💡 **中文翻译**：官方 Book 有高质量的中文社区翻译版 **[Rust 圣经 (Rust Course)](https://www.google.com/search?q=https://course.rs/)** 或 **[Rust 程序设计语言 (中文版)](https://kaisery.github.io/trpl-zh-cn/)**。

---

## GitHub 高星学习资源与项目

### 路线图与学习指南

* **[mikeroyal/Rust-Guide](https://www.google.com/search?q=https://github.com/mikeroyal/Rust-Guide)**
* 非常全面的 Rust 学习路线图与模块推荐，覆盖了 WebDev、GUI、GameDev、嵌入式等全栈指南。


* **[roadmap.sh/rust](https://www.google.com/search?q=https://github.com/kamranahmedse/developer-roadmap)**
* Developer Roadmap 官方推出的 Rust 开发者技能树图谱。



### 互动式练习与刷题

* **[rust-lang/rustlings](https://github.com/rust-lang/rustlings)** *(强烈推荐)*
* 官方出品的小练习集合！通过修补编译报错的小代码段来巩固 Rust 语法，上手效果极佳。


* **[rust-in-action/code](https://github.com/rust-in-action/code)**
* 《Rust 实战》一书的代码库，侧重系统级应用开发。



### 进阶与项目实战

* **[codecrafters-io/build-your-own-x](https://github.com/codecrafters-io/build-your-own-x)**
* 用 Rust 手把手实现复刻各种经典系统（如用 Rust 写 Redis、Git、Docker、操作系统等）。


* **[pingcap/talent-plan](https://github.com/pingcap/talent-plan)**
* PingCAP 开源的分布式系统与 Rust 进阶课程，包含完整的实验室练习（Lab）。


* **[notadle/awesome-rust](https://github.com/rust-unofficial/awesome-rust)**
* Rust 生态最全的资源汇总，包含了绝大多数优秀库、框架与工具链。



---

## 学习建议

1. **不要死磕理论**：Rust 编译器的提示非常友好（被戏称为“保姆级报错”），建议结合 `rustlings` 边写边被编译器打磨。
2. **理解 Lifetime 与 Borrow Checker**：初学者最常和编译器“斗智斗勇”，遇到生命周期报错时，优先思考数据所有权转移或使用引用/智能指针，而不是直接强制加上 `'a`。
3. **多看标准库源码**：Rust 的标准库代码风格极佳且文档完善，在 IDE 中随时跳转查看实现是提高 Rust 代码水平的捷径。