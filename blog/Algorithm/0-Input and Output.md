# 输入输出

# scanf  printf

- `scanf`和`printf`：适合格式化输出，速度较快。
- 不能取消同步流

# cin cout

- 方法灵活，取消同步流后速度也较快

~~~cpp
ios::sync_with_stdio(0), cin.tie(0), cout.tie(0); // 取消同步流
~~~

- std::endl 
  - 会强制刷新缓冲区

# 定义数组

尽量不要使用#define

~~~c
#define N (1e5 + 10)
const int N = 1e5 + 10
~~~



# 开数组

- 在main定义数组开栈空间
  - 栈空间大小一般为2M 4M 8M
  - 是否初始化（与编译器有关）

- 一般开全局

