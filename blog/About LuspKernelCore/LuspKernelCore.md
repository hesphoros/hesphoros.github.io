支撑上层所有模块的“底层基础库”
feature:
- 无任何业务逻辑
- 高复用
- 跨平台
- 稳定ABI
- 依赖少
# LuspKernelCore 实现路线规划（分阶段）

> 核心原则：优先实现“被依赖最多”的模块  
> 避免反复重构 / API 推翻 / 架构崩塌

---

# Phase 1：地基层（最优先）

## 目标
构建最基础能力，让后续模块有稳定依赖

## 模块

### 内存管理
- slab allocator（已完成）
- object pool
- small buffer optimization（已完成）

### 基础容器
- ring buffer（已完成）
- small vector（已完成）
- fixed queue
- intrusive list

### 并发基础（最小可用）
- mutex / spinlock（已完成）
- lock-free queue（已完成）
- thread pool（已完成）

说明：
暂不实现复杂调度

### 错误系统（必须优先）
- error_code
- error_category
- result<T, E>

### 基础工具
- 高精度 timer
- atomic 封装

---

## 阶段结果
- 可以编写稳定的多线程程序  
- 拥有基础内存与容器能力  

---

# Phase 2：系统抽象层

## 目标
实现跨平台基础 + IO 基础能力

## 模块

### 平台抽象
- FILE
- TIME（补全）
- THREAD
- ATOMIC（补全）

### IO 基础（重要）
- buffer
- descriptor

说明：
暂不实现 epoll / iocp

### 日志系统（基础版）
- level
- console sink
- async（可简化）

说明：
暂不实现 GUI / network / sqlite sink

---

## 阶段结果
- 具备工程级程序基础设施  
- 可支撑基础应用开发  

---

# Phase 3：工程能力层

## 目标
增强工程能力，形成基础框架能力

## 模块

### Configuration
- json（优先）
- 类型安全访问
- 热加载（可后续实现）

### 序列化
- binary（必须）
- json（复用 config）

说明：
暂不实现 msgpack

### 并发容器
- concurrent queue
- concurrent map（可选）

### task queue
- 与 thread pool 集成

---

## 阶段结果
- 可以开发完整工具 / 后端程序  
- 工程能力基本完善  

---

# Phase 4：核心能力（重点）

## 目标
构建高性能 IO 内核

## 模块

### IO 完整实现
- event loop
- epoll（优先）
- iocp / kqueue / io_uring（后续扩展）

建议顺序：
1. epoll
2. 抽象统一接口
3. 实现 iocp

---

### Event Bus
- publish / subscribe
- 异步分发

### Scheduler
- 定时任务
- 延迟执行
- 基于 timer + thread pool

---

## 阶段结果
- 具备高性能 IO 能力  
- 可支撑网络框架  

---

# Phase 5：高级架构层（后期）

## 目标
增强扩展性与系统架构能力

## 模块

### Module System
- 模块注册
- 生命周期管理（init / start / stop）
- 简单依赖注入

### Profiling
- 高精度计时
- RAII scope profiler

### Plugin（复杂）
- 动态加载（dll / so）
- 接口管理
- ABI 兼容

### Reflect（复杂度高）
- 类型注册
- 字段访问
- 自动序列化

说明：
建议后期再实现

---

### Security
- hash
- SHA256
- random

### Resource System
- 统一资源加载（file / memory / network）
- cache
- 生命周期管理

---

## 阶段结果
- 完整 runtime / framework 内核  
- 支持扩展与插件化  

---

# 测试（贯穿全程）

## 原则
测试不是阶段，而是持续行为

## 内容
- 单元测试（GTest）
- Mock
- Benchmark
- Test Helper

---

# 当前进度分析

## 已完成
- slab allocator
- small buffer optimization
- lock-free queue
- thread pool
- ring buffer

当前阶段：
Phase 2 到 Phase 3 之间

---

# 推荐执行顺序

## 下一步
1. error system
2. log（基础版）
3. platform / filesystem 补全
4. fixed queue / intrusive list

---

## 第二阶段
5. config（json）
6. serialize（binary + json）
7. task queue（完善）

---

## 第三阶段
8. IO（buffer + descriptor）
9. event loop（epoll）
10. scheduler

---

## 第四阶段
11. event bus
12. profiling

---

## 后期
13. module
14. plugin
15. reflect（慎重）

---

# 总结

## 正确路线
工具库 -> 工程库 -> IO 内核 -> runtime

## 注意事项
真正困难的部分在于：

- API 设计
- 模块边界
- 依赖方向

