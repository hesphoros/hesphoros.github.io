# 从 `socket()` 到 `brpc::Socket`：C++ 高性能网络库 Socket 源码剖析

这篇直接以 **Apache bRPC 当前 master** 为对象，不再停留在 POSIX Socket API 层面，而是回答一个更重要的问题：

> **一个普通的 Linux `socket(fd)`，究竟是怎么被 bRPC 包装成 `brpc::Socket`，最后变成一个能够承载 RPC、健康检查、连接复用、事件驱动和并发生命周期管理的对象？**

当前 bRPC 的 `Socket` 已经不是一个简单的 `fd` RAII wrapper。官方项目说明明确把它定位为底层连接抽象，负责 **fd 生命周期、SSL、写缓冲**，并通过 `VersionedRefWithId` 处理安全并发访问。([GitHub](https://github.com/apache/brpc/blob/master/CLAUDE.md?utm_source=chatgpt.com "brpc/CLAUDE.md at master · apache/brpc · GitHub"))

---

## 1. 先看整个调用链

先不要看代码，先建立一张地图。

```text
                         RPC Layer
                            │
                     Controller / Channel
                            │
                            ▼
                    InputMessenger
                            │
                            ▼
                    brpc::Socket
                 ┌──────────┼──────────┐
                 │          │          │
                 ▼          ▼          ▼
             InputBuf   OutputBuf    SSL
                 │          │
                 │          │
                 ▼          ▼
             EventDispatcher
                     │
                     ▼
                  epoll
                     │
                     ▼
               Linux Socket FD
                     │
                     ▼
                 TCP/IP
                     │
                     ▼
                    NIC
```

而从代码调用关系看，一个典型的接收路径可以简化成：

```text
epoll
  ↓
EventDispatcher
  ↓
Socket::ProcessEvent()
  ↓
InputMessenger::OnNewMessages()
  ↓
ProcessInputMessage()
  ↓
Protocol Parser
  ↓
RPC Handler
```

这个调用链并不是理论上的架构图。bRPC 实际的 crash/backtrace 中就可以看到：

```text
brpc::Socket::ProcessEvent
    ↓
brpc::InputMessenger::OnNewMessages
    ↓
brpc::ProcessInputMessage
    ↓
protocol::ProcessRpcResponse
```

例如 bRPC 的 issue #2925 中就出现了这一完整路径。([GitHub](https://github.com/apache/brpc/issues/2925?utm_source=chatgpt.com "内存暴涨下，sched_to 参数 pg 值被意外地修改 · Issue #2925 · apache/brpc · GitHub"))

这也是我们阅读 bRPC Socket 源码时最应该抓住的一条主线。

---

# 2. 第一层：Linux Socket

最底层仍然是：

```cpp
int fd = socket(AF_INET, SOCK_STREAM, 0);
```

Linux 内核中：

```text
fd
 │
 ▼
struct file
 │
 ▼
socket
 │
 ▼
sock
 │
 ├── TCP state
 ├── receive queue
 ├── send queue
 ├── callbacks
 └── protocol
```

如果自己写一个最简单的 TCP Server：

```cpp
int fd = socket(AF_INET, SOCK_STREAM, 0);

bind(fd, ...);
listen(fd, ...);

int client_fd = accept(fd, ...);
```

到这里还只是：

```text
C API
 +
Kernel Socket
```

而 bRPC 要解决的问题远远更多：

```text
fd 生命周期
连接状态
异步 I/O
事件分发
输入缓冲
输出缓冲
SSL
连接重建
健康检查
RPC 消息
并发访问
Socket 对象回收
```

因此 bRPC 必须在 `fd` 之上建立一个复杂对象。

---

# 3. `brpc::Socket` 并不是简单的 `Socket`

这是阅读源码前最重要的认识。

不要把：

```cpp
brpc::Socket
```

想象成：

```cpp
class Socket {
    int fd;
};
```

实际设计更接近：

```text
brpc::Socket
│
├── fd
├── endpoint
├── connection state
├── input state
├── output state
├── SSL state
├── callbacks
├── health-check state
├── reference/version state
└── event-dispatch state
```

因此：

> **bRPC 的 Socket 更像一个“连接对象生命周期管理中心”。**

而不是：

> “一个 FD 包装类”。

官方项目文档也明确把 Socket 描述为低层连接抽象，并指出它负责 fd 生命周期、SSL 和写缓冲，同时使用 `VersionedRefWithId` 做并发安全管理。([GitHub](https://github.com/apache/brpc/blob/master/CLAUDE.md?utm_source=chatgpt.com "brpc/CLAUDE.md at master · apache/brpc · GitHub"))

---

# 4. 为什么 bRPC 需要 `SocketId`？

这里开始进入 bRPC 最有意思的地方。

如果只有：

```cpp
int fd;
```

那么生命周期管理非常简单：

```text
create
 ↓
use
 ↓
close
 ↓
delete
```

但是高性能 RPC 框架里面：

```text
Thread A
   │
   │ 使用 Socket
   ▼
Socket

Thread B
   │
   │ Event
   ▼
Socket

Thread C
   │
   │ Health Check
   ▼
Socket

Thread D
   │
   │ RPC callback
   ▼
Socket
```

这时候 Socket 可能同时被多个执行流访问。

于是产生一个经典问题：

```text
Thread A                     Thread B

拿到 Socket*
    │
    │
    │                       close Socket
    │                            │
    │                            ▼
    │                         delete
    │
    ▼
访问 Socket
    │
    X
use-after-free
```

因此 bRPC 不能简单地：

```cpp
delete socket;
```

---

# 5. `VersionedRefWithId`

bRPC 为此引入：

```text
VersionedRefWithId<Socket>
```

这是理解 bRPC Socket 生命周期的关键组件之一。

可以把它先抽象成：

```text
              SocketId
                 │
                 ▼
        ┌─────────────────┐
        │ Socket Registry │
        ├─────────────────┤
        │ id → Socket     │
        └────────┬────────┘
                 │
                 ▼
              Socket
```

但是它不仅仅是：

```cpp
unordered_map<int, Socket*>
```

而是把：

```text
引用计数
+
版本
+
ID
+
回收
+
Revive
```

组合起来。

官方 bRPC 的当前文档明确提到 `Socket` 使用 `VersionedRefWithId` 做安全并发访问。([GitHub](https://github.com/apache/brpc/blob/master/CLAUDE.md?utm_source=chatgpt.com "brpc/CLAUDE.md at master · apache/brpc · GitHub"))

---

# 6. 为什么“版本号”很重要？

考虑一个非常危险的场景。

```text
Socket ID = 100
```

线程 A：

```text
拿到：
SocketId = 100
```

随后：

```text
Socket 100
 ↓
destroy
```

然后系统又创建了一个新的 Socket：

```text
Socket ID = 100
```

如果只有：

```text
ID = 100
```

线程 A 很可能把：

```text
旧 Socket 100
```

误认为：

```text
新 Socket 100
```

这就是典型的：

> ABA 问题。

所以需要：

```text
ID
+
Version
```

例如：

```text
100@7
```

和：

```text
100@8
```

虽然 ID 相同：

```text
100
```

但是版本不同。

于是：

```text
100@7
≠
100@8
```

---

# 7. 这就是为什么 bRPC Socket 看起来非常“重”

普通 Socket：

```text
fd = 42
```

bRPC Socket：

```text
SocketId
   │
   ├── id
   └── version

Socket object
   │
   ├── fd
   ├── state
   ├── references
   ├── callbacks
   ├── buffers
   └── transport state
```

因此阅读：

```text
src/brpc/socket.h
src/brpc/socket.cpp
src/brpc/versioned_ref_with_id.h
```

实际上应该一起读。

---

# 8. Socket 创建

从概念上：

```text
Socket::Create
       │
       ▼
allocate Socket
       │
       ▼
initialize state
       │
       ▼
create/register fd
       │
       ▼
register EventDispatcher
       │
       ▼
return SocketId
```

这里特别重要：

> **Socket 的“对象创建”和“fd 创建”不是一个概念。**

可以出现：

```text
Socket object
     │
     └── fd
```

但是：

```text
Socket object
```

本身还拥有大量 bRPC 状态。

---

# 9. EventDispatcher

接下来进入 bRPC 网络层的另一个核心：

```text
EventDispatcher
```

它解决的是：

> “哪个 Socket 现在有事件？”

普通程序可能直接：

```cpp
epoll_wait(...)
```

然后：

```cpp
recv(fd, ...)
```

bRPC 则进一步抽象：

```text
                 EventDispatcher
                       │
                    epoll
                       │
        ┌──────────────┼──────────────┐
        ▼              ▼              ▼
      fd 10          fd 20          fd 30
        │              │              │
       Socket         Socket         Socket
```

EventDispatcher 并不负责 RPC 协议。

它主要关心：

```text
Readable
Writable
Error
Close
```

然后把事件交给对应 Socket。

---

# 10. 为什么不让 Socket 自己调用 epoll？

因为职责应该分离。

如果每个 Socket 都：

```cpp
epoll_create();
epoll_wait();
```

那么：

```text
10000 Socket
   ↓
10000 epoll
```

显然是不合理的。

更合理：

```text
               EventDispatcher
                      │
                      ▼
                    epoll
                      │
       ┌──────────────┼──────────────┐
       ▼              ▼              ▼
    Socket A       Socket B       Socket C
```

一个 EventDispatcher 管理很多 Socket。

这就是 Reactor 的基本思想。

---

# 11. Socket 注册到 EventDispatcher

可以把它抽象成：

```text
Socket
  │
  │ fd
  ▼
EventDispatcher
  │
  │ epoll_ctl
  ▼
epoll
```

例如：

```text
fd = 42
```

注册：

```text
epoll
 └── fd 42
      └── callback/context
```

所以当：

```text
NIC
 ↓
TCP
 ↓
Receive Buffer
 ↓
fd 42 readable
```

内核通知：

```text
epoll_wait()
```

之后 EventDispatcher 找到：

```text
fd 42
 ↓
Socket
```

再调用：

```cpp
Socket::ProcessEvent(...)
```

---

# 12. `Socket::ProcessEvent()`

这是整个分析中最关键的函数之一。

我们可以把它理解成：

```text
epoll Event
      │
      ▼
Socket::ProcessEvent()
      │
      ├── readable?
      │
      ├── writable?
      │
      ├── error?
      │
      └── close?
```

例如：

```text
EPOLLIN
```

表示：

```text
Socket 可以读取数据
```

于是：

```text
Socket::ProcessEvent()
        │
        ▼
InputMessenger
```

而：

```text
EPOLLOUT
```

则意味着：

```text
Socket 可以继续发送数据
```

于是：

```text
Socket
  ↓
OutputBuffer
  ↓
send()
```

---

# 13. 从这里进入 InputMessenger

当前 bRPC 源码体系里：

```text
Socket
 ↓
InputMessenger
```

是非常重要的一层。

为什么不直接：

```cpp
recv(fd)
```

？

因为：

```text
TCP byte stream
```

并不等于：

```text
RPC message
```

所以必须有：

```text
Socket
 ↓
read bytes
 ↓
InputMessenger
 ↓
message framing
 ↓
protocol parser
```

---

# 14. `InputMessenger::OnNewMessages()`

典型路径：

```text
Socket::ProcessEvent
        │
        ▼
InputMessenger::OnNewMessages
        │
        ▼
read from socket
        │
        ▼
Input Buffer
        │
        ▼
find complete message
        │
        ▼
ProcessInputMessage
```

这也可以从真实 bRPC 的调用栈验证。

例如官方 issue #2925 的堆栈中：

```text
Socket::ProcessEvent
 ↓
InputMessenger::OnNewMessages
 ↓
ProcessInputMessage
 ↓
policy::ProcessRpcResponse
```

([GitHub](https://github.com/apache/brpc/issues/2925?utm_source=chatgpt.com "内存暴涨下，sched_to 参数 pg 值被意外地修改 · Issue #2925 · apache/brpc · GitHub"))

这条调用链非常重要，建议你以后看 bRPC crash 时第一时间认出它。

---

# 15. 为什么 Socket 不负责解析 RPC？

因为 bRPC 支持多个协议。

官方当前项目介绍列出的协议包括：

```text
baidu_std
HTTP/H2
gRPC
thrift
redis
memcached
RTMP
RDMA
```

等。([GitHub](https://github.com/apache/brpc/blob/master/CLAUDE.md?utm_source=chatgpt.com "brpc/CLAUDE.md at master · apache/brpc · GitHub"))

所以：

```text
Socket
```

不能知道：

```text
这是 HTTP？
这是 gRPC？
这是 Redis？
这是 baidu_std？
```

它只负责：

```text
bytes
```

因此：

```text
Socket
 ↓
InputMessenger
 ↓
Protocol
```

才是合理架构。

---

# 16. IOBuf：bRPC 的关键 Buffer

到了这里必须引入：

```text
butil::IOBuf
```

它是 bRPC 非常重要的数据结构。

官方项目文档把 `IOBuf` 描述为：

> zero-copy buffer

其内部采用引用计数 block，并使用 SmallView / BigView 优化。([GitHub](https://github.com/apache/brpc/blob/master/CLAUDE.md?utm_source=chatgpt.com "brpc/CLAUDE.md at master · apache/brpc · GitHub"))

这意味着：

```text
Socket
 ↓
IOBuf
 ↓
Protocol
```

而不是简单：

```cpp
char buffer[4096];
```

---

# 17. 为什么需要 IOBuf？

假设一个 RPC 请求：

```text
10 MB
```

如果：

```text
recv()
 ↓
char buffer
 ↓
memcpy
 ↓
protocol buffer
 ↓
protobuf
```

可能产生大量数据复制。

理想情况：

```text
NIC
 ↓
Kernel
 ↓
IOBuf
 ↓
Protocol
```

尽可能减少：

```text
memcpy
```

---

# 18. IOBuf 的思想

可以粗略理解为：

```text
IOBuf
 │
 ├── Block A
 ├── Block B
 ├── Block C
 └── Block D
```

而不是要求：

```text
一整块连续内存
```

因此：

```text
             IOBuf
        ┌──────┼──────┐
        ▼      ▼      ▼
      Block  Block  Block
```

可以通过引用计数共享。

这对于：

```text
RPC
Streaming
Zero-copy
RDMA
```

都非常重要。

bRPC 的 RDMA 文档也特别强调 IOBuf 内存池以及零拷贝路径。([GitHub](https://github.com/apache/brpc/blob/master/docs/cn/rdma.md?utm_source=chatgpt.com "brpc/docs/cn/rdma.md at master · apache/brpc · GitHub"))

---

# 19. Output Path

输入方向：

```text
Network
 ↓
Socket
 ↓
InputMessenger
 ↓
IOBuf
 ↓
Protocol
```

输出方向正好反过来：

```text
RPC Response
 ↓
IOBuf
 ↓
Socket
 ↓
Output Buffer
 ↓
send()
 ↓
Kernel
 ↓
NIC
```

关键问题来了：

> 如果 `send()` 只发送了一部分怎么办？

答案：

```text
Output Buffer
```

例如：

```text
IOBuf
 │
 ▼
Socket
 │
 ├── send 64KB
 │
 └── remaining 936KB
          │
          ▼
      Output Buffer
```

等下次：

```text
EPOLLOUT
```

再继续发送。

---

# 20. 为什么 `EPOLLOUT` 不能一直监听？

因为绝大多数时候 TCP Socket 都是：

```text
writable
```

如果一直监听：

```text
EPOLLOUT
```

EventLoop 会疯狂收到：

```text
writable
writable
writable
writable
...
```

导致 CPU 空转。

因此高性能网络库通常：

```text
没有待发送数据
      ↓
不监听 EPOLLOUT

有待发送数据
      ↓
开启 EPOLLOUT

发送完成
      ↓
关闭 EPOLLOUT
```

这也是 Reactor 网络库的重要优化。

---

# 21. Socket 生命周期才是最难的部分

现在我们开始进入 bRPC 真正复杂的地方。

一个 Socket 可能经历：

```text
NEW
 ↓
CONNECTING
 ↓
CONNECTED
 ↓
READABLE
 ↓
WRITING
 ↓
ERROR
 ↓
CLOSED
```

但同时还有：

```text
RPC reference
Health Check reference
Event reference
Callback reference
SocketMap reference
```

因此真正的生命周期更像：

```text
                   Socket
                     │
        ┌────────────┼────────────┐
        │            │            │
       RPC        Event       HealthCheck
        │            │            │
        └────────────┼────────────┘
                     │
                Reference
                     │
                     ▼
                  Release
                     │
                     ▼
                  Recycle
```

---

# 22. `HoldHCRelatedRef()` 就是一个典型例子

当前 bRPC 源码附近存在：

```cpp
HoldHCRelatedRef()
ReleaseHCRelatedReference()
```

这说明：

> Health Check 本身也参与 Socket 生命周期管理。

官方 issue #2998 就直接涉及这两个函数以及 `AddReference()` / `Dereference()` 的行为。([GitHub](https://github.com/apache/brpc/issues/2998?utm_source=chatgpt.com "未开启健康检查时客户端偶发崩溃且易现一直超时 · Issue #2998 · apache/brpc · GitHub"))

这也说明为什么你不能把：

```text
Socket
```

简单理解成：

```text
fd
```

---

# 23. Health Check 为什么需要持有 Socket？

假设：

```text
RPC Thread
    │
    ▼
Socket
```

与此同时：

```text
Health Check Thread
    │
    ▼
Socket
```

如果 RPC 线程释放最后一个引用：

```text
Socket
 ↓
delete
```

Health Check 线程怎么办？

```text
Health Check
     │
     ▼
dangling pointer
```

所以：

```text
Health Check
     │
     ▼
AddReference()
     │
     ▼
Socket
```

任务结束：

```text
Dereference()
```

---

# 24. 这也是 bRPC Socket 最难理解的地方

普通网络库：

```text
Socket
 ↓
close
 ↓
delete
```

bRPC：

```text
Socket
 │
 ├── EventDispatcher
 ├── InputMessenger
 ├── RPC
 ├── HealthCheck
 ├── Channel
 └── Callback
       │
       ▼
   Reference Graph
       │
       ▼
   VersionedRefWithId
       │
       ▼
   Safe Recycle
```

所以如果你之后阅读：

```text
socket.cpp
```

发现大量：

```cpp
AddReference()
Dereference()
Reset()
Revive()
WaitAndReset()
```

不要觉得这些代码是“复杂化”。

它们实际上是在解决：

> **一个高度并发的 Socket 对象如何安全死亡。**

---

# 25. `Revive()` 是什么？

这是 bRPC Socket 一个非常有意思的机制。

普通 Socket：

```text
连接断开
 ↓
destroy
```

bRPC 更倾向于：

```text
Socket
 ↓
connection broken
 ↓
Reset / Check
 ↓
Revive
 ↓
reconnect
```

例如：

```text
Socket #1280
192.168.1.10:8000
```

连接断开：

```text
ERROR
```

健康检查发现：

```text
server is available
```

然后：

```text
Revive
```

重新建立连接。

bRPC issue #671 中的实际日志就能看到：

```text
Checking SocketId=1280
...
Revived SocketId=1280
```

以及此前的：

```text
Connection timed out
Broken pipe
```

([GitHub](https://github.com/apache/brpc/issues/671?utm_source=chatgpt.com "Client与Server出现网络断开25秒，期间一直无法连接 · Issue #671 · apache/brpc · GitHub"))

所以：

> **Socket 对象本身可以比一次 TCP connection 活得更久。**

这个思想非常重要。

---

# 26. Socket ≠ TCP Connection

这是阅读 bRPC 时必须建立的新认知。

普通程序：

```text
Socket
=
TCP Connection
```

而 bRPC：

```text
brpc::Socket
     │
     ├── 当前 TCP connection
     │
     ├── endpoint
     ├── buffers
     ├── references
     ├── health check
     └── reconnect state
```

因此：

```text
TCP connection dead
```

并不一定意味着：

```text
brpc::Socket object dead
```

可能只是：

```text
TCP Connection
     ↓
dead
     ↓
Socket
     ↓
Revive
     ↓
new TCP Connection
```

---

# 27. 这对 RPC 很重要

例如：

```cpp
channel.CallMethod(...)
```

Channel 需要的是：

```text
一个可用的 endpoint
```

而不是：

```text
某个永远不变的 fd
```

所以：

```text
Channel
 ↓
SocketMap
 ↓
Socket
 ↓
connection
```

当连接失败：

```text
connection dead
```

可以：

```text
Socket revive
```

而不是让上层所有 RPC 对象全部重建。

---

# 28. 真正的接收链

现在把整个输入路径完整串起来。

```text
               NIC
                │
                ▼
             Ethernet
                │
                ▼
               TCP
                │
                ▼
        Kernel Receive Buffer
                │
                ▼
              epoll
                │
                ▼
        EventDispatcher
                │
                ▼
       Socket::ProcessEvent
                │
                ▼
       InputMessenger
                │
                ▼
              IOBuf
                │
                ▼
        Message Framing
                │
                ▼
       ProcessInputMessage
                │
                ▼
        Protocol Decoder
                │
                ▼
         RPC Controller
                │
                ▼
        User RPC Handler
```

这才是：

> 一次网络请求从网卡进入 C++ RPC Handler 的完整路径。

---

# 29. 真正的发送链

反过来：

```text
User RPC Handler
       │
       ▼
RPC Response
       │
       ▼
     IOBuf
       │
       ▼
    Socket
       │
       ▼
 Output Buffer
       │
       ▼
    send()
       │
       ▼
Kernel TCP Send Buffer
       │
       ▼
      TCP
       │
       ▼
      NIC
       │
       ▼
    Network
```

如果发送不完：

```text
Output Buffer
      │
      ▼
EPOLLOUT
      │
      ▼
continue send()
```

---

# 30. bthread 在哪里？

到目前为止，我们说的是：

```text
epoll
 ↓
Socket
```

但是 bRPC 还有一个非常重要的组成部分：

```text
bthread
```

官方项目说明明确指出，bRPC 的大多数 callback 在 **bthread** 中执行，而不是普通 pthread。([GitHub](https://github.com/apache/brpc/blob/master/CLAUDE.md?utm_source=chatgpt.com "brpc/CLAUDE.md at master · apache/brpc · GitHub"))

所以实际路径更接近：

```text
epoll
 ↓
EventDispatcher
 ↓
Socket::ProcessEvent
 ↓
schedule bthread
 ↓
bthread worker
 ↓
InputMessenger
 ↓
RPC
```

这就是 bRPC 与一个简单 epoll Server 的巨大区别。

---

# 31. 为什么 bRPC 使用 bthread？

假设：

```text
10000 RPC
```

如果：

```text
10000 pthread
```

成本很高。

bRPC 使用 M:N 风格的用户态线程调度：

```text
             bthreads
     ┌────┬────┬────┬────┐
     │ B1 │ B2 │ B3 │ ...│
     └─┬──┴─┬──┴─┬──┴────┘
       │    │    │
       └────┼────┘
            ▼
       pthread workers
```

因此：

```text
很多 bthread
        ↓
少量 pthread
```

可以获得更轻量的并发模型。

---

# 32. 一个真实的 bRPC 调用栈

例如官方 issue 中出现过这样的栈：

```text
bthread_make_fcontext
    ↓
bthread::TaskGroup::task_runner
    ↓
brpc::Socket::ProcessEvent
    ↓
brpc::InputMessenger::OnNewMessages
    ↓
brpc::ProcessInputMessage
    ↓
brpc::policy::ProcessRpcResponse
    ↓
Controller::OnResponse
    ↓
Controller::EndRPC
```

([GitHub](https://github.com/apache/brpc/issues/2925?utm_source=chatgpt.com "内存暴涨下，sched_to 参数 pg 值被意外地修改 · Issue #2925 · apache/brpc · GitHub"))

这一条栈非常值得记下来。

它实际上告诉我们：

```text
Network Event
     ↓
Socket
     ↓
Input Messenger
     ↓
Protocol
     ↓
RPC Controller
     ↓
User Callback
```

---

# 33. 为什么 Socket 里面会出现很多并发问题？

因为它实际上处于整个网络栈的中心：

```text
             Channel
                │
                ▼
            SocketMap
                │
                ▼
             Socket
          ┌─────┼─────┐
          │     │     │
          ▼     ▼     ▼
        Event  RPC   HC
          │     │     │
          └─────┼─────┘
                ▼
            bthread
```

所以：

```text
Socket
```

天然是并发热点。

这也是为什么近年来 bRPC issue 中仍然能看到围绕 Socket 生命周期、并发和 use-after-free 的问题。比如 issue #3165 的 ASAN 栈就涉及：

```text
Socket::AfterRevived()
VersionedRefWithId<Socket>::Revive()
HealthCheckTask
```

([GitHub](https://github.com/apache/brpc/issues/3165?utm_source=chatgpt.com "使用ASAN profile显示 logging 有 heap-use-after-free · Issue #3165 · apache/brpc · GitHub"))

---

# 34. 一个非常典型的 UAF 场景

假设：

```text
Thread A:
    Socket::ProcessEvent()

Thread B:
    Socket::Revive()

Thread C:
    Socket::Dereference()
```

可能形成：

```text
              Socket
                │
       ┌────────┼────────┐
       │        │        │
       ▼        ▼        ▼
     Event    Revive   Deref
       │        │        │
       │        │        ▼
       │        │      ref--
       │        │
       │        ▼
       │      reset
       │
       ▼
   ProcessEvent
```

如果引用管理不正确：

```text
delete
  ↓
ProcessEvent
  ↓
use-after-free
```

这正是：

```text
VersionedRefWithId
+
reference counting
+
generation/version
```

存在的原因。

---

# 35. bRPC Socket 的真正职责

到这里，可以重新定义：

### 普通 Socket

```text
Socket
=
FD
```

### bRPC Socket

```text
Socket
=
Connection Lifecycle Manager
+
I/O Endpoint
+
Event Handler
+
Buffer Owner
+
Health Check Target
+
Reference Managed Object
+
Transport Abstraction
```

这才是正确的理解方式。

---

# 36. 更有意思的是：Socket 甚至不一定是 TCP

这一点非常容易被忽略。

bRPC 当前已经不仅仅支持 TCP。

官方 RDMA 文档明确说明：

> RDMA 不使用 socket 接口进行通信，但 bRPC 仍然复用了原来的 `Socket` 类。

在 RDMA 模式下：

```text
Socket
   │
   └── RdmaEndpoint
```

写数据时：

```text
Socket
 ↓
RdmaEndpoint
 ↓
RDMA QP
 ↓
verbs
```

而不是：

```text
Socket
 ↓
fd
 ↓
send()
```

([GitHub](https://github.com/apache/brpc/blob/master/docs/cn/rdma.md?utm_source=chatgpt.com "brpc/docs/cn/rdma.md at master · apache/brpc · GitHub"))

---

# 37. 这说明 Socket 是一个“Transport Endpoint”

因此更准确的抽象：

```text
                 brpc::Socket
                       │
             ┌─────────┴─────────┐
             │                   │
            TCP                RDMA
             │                   │
            fd                 QP
             │                   │
          epoll               CQ
```

也就是说：

> bRPC 的 Socket 已经从“Linux Socket”抽象成了“网络传输端点”。

这是非常重要的架构升级。

---

# 38. TCP 和 RDMA 为什么还能共用 Socket？

因为上层只关心：

```text
read
write
connection
event
```

而不想关心：

```text
TCP fd
```

还是：

```text
RDMA QP
```

因此：

```text
                  Socket
                    │
          ┌─────────┴─────────┐
          ▼                   ▼
    Tcp Transport        RDMA Endpoint
          │                   │
         fd                  QP
```

这就是抽象层的价值。

---

# 39. 这也解释了 `SocketMode`

当前 bRPC 的 transport 架构已经扩展到：

```text
TCP
RDMA
UBSHM
URMA
```

近期 bRPC 社区讨论和代码变更也在继续扩展 transport abstraction。相关设计中可以看到 `SocketMode` 与 `TransportFactory` 被用于选择不同传输实现。([GitHub](https://github.com/apache/brpc/issues/3401?utm_source=chatgpt.com "基于URMA 的远程内存语义传输层 · Issue #3401 · apache/brpc · GitHub"))

所以未来：

```text
brpc::Socket
```

越来越不能理解成：

```text
Linux fd wrapper
```

而应该理解成：

```text
Generic transport endpoint
```

---

# 40. 最终架构图

现在把所有东西合在一起：

```text
                         RPC
                          │
                 ┌────────▼────────┐
                 │    Controller   │
                 └────────┬────────┘
                          │
                     Channel
                          │
                          ▼
                    SocketMap
                          │
                          ▼
                  ┌───────────────┐
                  │ brpc::Socket  │
                  └───────┬───────┘
                          │
            ┌─────────────┼─────────────┐
            │             │             │
            ▼             ▼             ▼
       InputMessenger  OutputBuffer  HealthCheck
            │             │
            ▼             ▼
          IOBuf         IOBuf
            │             │
            └──────┬──────┘
                   ▼
            EventDispatcher
                   │
              ┌────┴────┐
              ▼         ▼
            epoll      RDMA CQ
              │         │
              ▼         ▼
             TCP       RDMA
              │         │
              ▼         ▼
             NIC       NIC
```

而并发层：

```text
                 bthread
                    │
          ┌─────────┼─────────┐
          ▼         ▼         ▼
      RPC Task   Event Task  HC Task
          │         │         │
          └─────────┼─────────┘
                    ▼
              brpc::Socket
                    │
                    ▼
          VersionedRefWithId
```

---

# 41. 阅读源码应该按照什么顺序？

如果你现在已经开始真正读 bRPC，我**不建议直接从 `socket.cpp` 第一行开始读**。

非常容易迷路。

推荐这个顺序：

```text
① socket.h
       ↓
② socket.cpp
       ↓
③ versioned_ref_with_id.h
       ↓
④ event_dispatcher.h/.cpp
       ↓
⑤ input_messenger.h/.cpp
       ↓
⑥ iobuf.h/.cpp
       ↓
⑦ socket_map.h/.cpp
       ↓
⑧ channel.cpp
       ↓
⑨ tcp_transport
       ↓
⑩ bthread
```

其中前三个尤其重要：

```text
Socket
VersionedRefWithId
EventDispatcher
```

---

# 42. 第一阶段：先研究 `Socket` 的数据成员

不要急着看函数。

打开：

```text
src/brpc/socket.h
```

先把：

```cpp
class Socket
```

里面所有成员变量分类。

建议自己整理成：

```text
Socket
│
├── Identity
│   ├── SocketId
│   └── version
│
├── OS
│   └── fd
│
├── Endpoint
│   ├── local
│   └── remote
│
├── Input
│   ├── InputMessenger
│   └── read state
│
├── Output
│   ├── write buffer
│   └── write state
│
├── Lifecycle
│   ├── ref
│   ├── state
│   └── reset
│
├── HealthCheck
│
└── Transport
    ├── TCP
    └── RDMA
```

做完这个，你会突然发现：

> Socket.cpp 没那么可怕了。

---

# 43. 第二阶段：只追一条函数链

第一次不要研究所有功能。

只追：

```text
epoll event
   ↓
Socket::ProcessEvent()
   ↓
InputMessenger::OnNewMessages()
   ↓
ProcessInputMessage()
```

然后回答四个问题：

```text
1. 谁调用 ProcessEvent？
2. ProcessEvent 如何知道 fd 有数据？
3. OnNewMessages 如何读取数据？
4. 读取的数据如何变成 RPC Message？
```

这四个问题搞清楚：

> bRPC 的网络输入路径基本就通了。

---

# 44. 第三阶段：研究 Socket 为什么不会被随便 delete

然后专门研究：

```text
Create
AddReference
Dereference
Reset
WaitAndReset
Revive
AfterRevived
```

再研究：

```text
VersionedRefWithId
```

这时候你会开始真正理解 bRPC 的并发模型。

---

# 45. 第四阶段：研究 Output Path

然后：

```text
send
 ↓
write buffer
 ↓
EPOLLOUT
 ↓
continue write
```

重点研究：

```text
为什么要 buffer？
什么时候注册 EPOLLOUT？
什么时候取消 EPOLLOUT？
IOBuf 如何进入 Socket？
```

---

# 46. 第五阶段：再研究 RDMA

最后才看：

```text
Socket
 ↓
RdmaEndpoint
 ↓
QP
 ↓
CQ
```

因为你已经知道：

```text
Socket
InputMessenger
EventDispatcher
IOBuf
```

以后再看 RDMA，就会发现：

> bRPC 实际上在尽量复用上层消息处理框架。

官方文档也明确说明 RDMA 最终仍复用 `Socket` 与 `InputMessenger`。([GitHub](https://github.com/apache/brpc/blob/master/docs/cn/rdma.md?utm_source=chatgpt.com "brpc/docs/cn/rdma.md at master · apache/brpc · GitHub"))

---

# 47. 这篇最重要的结论

如果只记住几个东西，我建议记住下面这张图：

```text
Linux socket()
      │
      ▼
     fd
      │
      ▼
EventDispatcher
      │
      ▼
 epoll event
      │
      ▼
brpc::Socket
      │
      ├──────────────┐
      │              │
      ▼              ▼
InputMessenger   OutputBuffer
      │              │
      ▼              ▼
    IOBuf           IOBuf
      │              │
      ▼              ▼
Protocol          send()
      │
      ▼
 RPC
```

而 `brpc::Socket` 本身又被：

```text
VersionedRefWithId
```

保护生命周期。

因此最终应该形成这样一个认识：

> **Linux Socket 是一个 FD + Kernel 网络对象；bRPC Socket 则是在它之上构建的一个带有事件驱动、缓冲区、协议入口、健康检查、引用生命周期和多传输后端的“连接端点对象”。**

这也是为什么你从普通：

```cpp
socket()
```

一路读到：

```cpp
brpc::Socket
```

以后，会发现中间其实隔着**一整套高性能网络框架设计**。

---

### 建议的下一篇

如果继续按照“源码考古”的方式读，我建议下一篇直接深入：

**《`brpc::Socket` 源码第一层：`VersionedRefWithId` 是怎么解决 Socket 生命周期与 ABA/UAF 问题的？》**

这一篇可以直接围绕：

```text
VersionedRefWithId<T>
        ↓
Create()
        ↓
AddReference()
        ↓
Dereference()
        ↓
Reset()
        ↓
WaitAndReset()
        ↓
Revive()
        ↓
AfterRevived()
```

把 **SocketId、version、reference counting、对象回收、Revive、HealthCheck 并发关系** 全部扒开。这个部分实际上比 `epoll` 本身更值得研究，也是理解 bRPC 源码最关键的一关。 bRPC 当前源码和实际 issue 中都能看到这套机制与 Socket 崩溃/UAF/健康检查问题直接相关。([GitHub](https://github.com/apache/brpc/issues/3165?utm_source=chatgpt.com "使用ASAN profile显示 logging 有 heap-use-after-free · Issue #3165 · apache/brpc · GitHub"))

[Apache bRPC 源码仓库](https://github.com/apache/brpc?utm_source=chatgpt.com)