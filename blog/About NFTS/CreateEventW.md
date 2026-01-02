# CreateEventW

创建或打开命名或未命名的事件对象。

 lpEventAttributes：指向`SECURITY_ATTRIBUTES`结构体，此结构体决定函数的返回句柄是否可以让子进程继承。如果这个参数为NULL，这个句柄是不能继承的。一般情况下，这个参数设置为NULL。

 bManualReset：指定将创建的EVENT是自动复位还是手动复位。如果为TRUE，需要用ResetEvent(HANDLE)函数手动复位状态为无信号，即一旦改EVENT被设置成有信号，则它会一直等到ResetEvent调用时才为无信号状态。如果为FALSE，当一个有信号的等待线程被释放后，系统会自动复位状态为无信号状态。

 bInitialState：指定事件对象的初始状态。如果为TRUE，初始状态为有信号，否则为无信号。

 lpName：  事件对象的名称，以字符串表示。名称的长度受MAX_PATH的限制，名称是大小写敏感的。如果lpName匹配一个存在的命名的事件对象，函数将请求EVENT_ALL_ACCESS来访问存在的对象。在这种情况下，bManualReset和bInitialState 被忽略，因为这两个参数已经被存在的事件设置。如果lpEventAttributes参数不为NULL，这个参数可以决定是否句柄被继承，但是它的安全描述（security-descriptor）成员被忽略。如果lpName 为NULL，创建一个没有名称的事件。如果lpName 匹配一个存在的semaphore, mutex, waitable timer, job或者file-mapping对象的名称，函数调用失败，GetLastError函数返回ERROR_INVALID_HANDLE。由于这些对象共享相同的命名空间，才导致这种情况的发生。

 返回值：    函数返回句柄，该句柄具有`EVENT_ALL_ACCESS`权限去访问新的事件对象，同时它可以在任何需要事件对象句柄的函数中使用。