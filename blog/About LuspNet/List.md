此文档是为编写LuspNet网络、I/O库的参照来源

# 两大主流模型

1. Reactor 事件驱动模型
	-  比如： libevent libuv boost.asio
2. Proactor 完成驱动模型
	-  比如： Windows IOCP、Linux io_uring

阅读书籍：

操作系统 I/O：
UNIX Network Programming
Linux System Programming
Windows Internal 4editon
muduo 设计

内存模型+多线程
C++ Concurrency in Action

poller 比如epoll的封装 
channel
buffer 高性能buffer
timer 
线程模型 



