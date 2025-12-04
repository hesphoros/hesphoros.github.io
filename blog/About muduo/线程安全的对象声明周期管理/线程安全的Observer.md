# 线程安全的Observer有多难

此处给出一个典型的Observer模式: 

```cpp

class Observer
{
 public:
  virtual ~Observer();
  virtual void update() = 0;
  void observe(Observable* s);
 protected:
  Observable* subject_;
};

class Observable
{
 public:
  void register_(Observer* x);
  void unregister(Observer* x);
  
  void notifyObservers()
  {
    for (Observer* o : observers_)
    {
      if (o)
        o->update(); //( 3 )
    }
  }

 private:
  std::vector<Observer*> observers_;
};

```


  > (3) 在通知每一个观察者时 从何得知Observer对象o是否还存活? 
  >
  > 	一个动态创建的对象是否还活着,光看指针/引用是看不出来的
  >								
  > 	指针就是指向了一块内存.如果这块内存上的对象已经销毁
  >								
  > 	那么就不可能访问,既然不能访问如何又知道对象的状态呢? 
  >
  > **判断一个指针是不是合法的指针没有效的办法? 万一这个新的对象的类型异于老的对象呢?**


如果在Observer的析构函数里调用unregister()来接注册? 恐难凑效

```cpp

Observer::~Observer()
{
  subject_->unregister(this);
}

void Observer::observe(Observable* s)
{
  s->register_(this);
  subject_ = s;
}

```

此处析构函数去调用两个race conditions

1. 如何得知`subject`_还存活?
2. 就算subject_指向某个永久存在的对象 那么如果:
	1. `ThreadA` 执行到 `subject_->unregister(this);` 之前 ,还没有来得及`unregester`本对象
	2. `ThreadB` 执行到 o->update ,o 正好指向是ThreadA正在析构的对象

```bash
线程A (析构 Observer)            线程B (通知 Observers)
─────────────────────          ─────────────────────
Observer::~Observer() {        notifyObservers() {
                                   for (Observer* o : observers_) {
  // 还没来得及 unregister
                                    o->update();  // o 指向正在析构的对象！
  subject_->unregister(this);                     // 灾难！
}                               }
```



>   此时,既然o指向的Observer对象正在析构,调用他的任何非静态成员函数都是不安全的  何况是虚函数 ,
>   
>   而且Observer是个基类,执行 subject_->unregister(this); 的时候  派生类已经析构了, 此时整个对象处于将死未死的状态,调用虚函数更是危险.



> 派生类析构:
>
> 1. 先调用子类的析构函数
> 2. 再调用父类的析构函数


# 原始指针有何不妥

在`Observable`中持有了observers_的raw point

```cpp

class Observable
{

 private:
  
  // 原始指针的核心问题: 
  //它只是一个内存地址, 无法告诉你这个地址上的对象是否还存活
  std::vector<Observer*> observers_;
};
```

使用Observer*的指针时:
  1. 你不知道它指向的对象是否已经被delete了
   2. 你不知道它指向的内存是否被重新分配给了其他对象
   3. 访问一个已经被delete的对象是未定义行为

## subject_不能是原始指针

问题在于：你怎么知道 subject_ 指向的 Observable 还活着？

```cpp

Observer::~Observer()
{
  subject_->unregister(this);  // 危险！subject_ 可能已经被销毁了！
}

```

考虑场景: 

> 时间线：
> T1: Observable subject 被销毁
> T2: Observer observer 被销毁，调用 subject_->unregister(this)
>     —— 此时 subject_ 是野指针！访问它是未定义行为！



## `shared_ptr`的循环引用问题

```c++
class Observer {
  // Observer 持有 Observable
  std::shared_ptr<Observable> subject_;  
};

class Observable {
  // Observable 持有 Observer
  std::vector<std::shared_ptr<Observer>> observers_;
};
```

这就形成了循环引用:

```
Observer ──shared_ptr──> Observable
    ^                         │
    └────shared_ptr───────────┘
```

两者互相持有对方，引用计数永远不会归零，**内存永远不会释放**！

## 正确的解决方案

使用`weak_ptr`

```c++
class Observable {
  // 弱引用，不增加引用计数
  std::vector<std::weak_ptr<Observer>> observers_;  
};
```

`weak_ptr` 的特点：

- 不增加引用计数（不会造成循环引用）
- 可以通过 `lock()` 检查对象是否还活着
- 如果对象已销毁，`lock()` 返回空的 `shared_ptr`

这样 `notifyObservers()` 就可以安全地检查：

```c++
void notifyObservers() {
  for (auto& weak_o : observers_) {
    auto o = weak_o.lock();  // 尝试获取 
    if (o) {                 // 如果对象还活着
      o->update();
    }
  }
}
```

# 空悬指针 (Dangling Pointer)

空悬指的是**指向一个已经无效或被释放的内存地址的指针**。

当指针所指向的内存不再属于该程序时，这个指针就变成了“空悬”的，因为它指向的悬空（Dangling）的、不确定的空间



有两个指针p1 和 p2,指向堆上同一对象object, p1 和 p2在不同的线程中 (左图)

假如线程A通过p1指针将对象销毁了,尽管p1被置为NULL,那么p2就成了空悬指针.

![image-20251203202940885](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20251203202940885.png)

所以,要想要安全的回收对象,最好是在别的线程都看不到的情况下,偷偷的做
(**这就是垃圾回收的原理,所有人都用不到的东西一定是垃圾**)

## 一个"解决办法"

引入一层间接性,让p1和p2所指的对象永久有效. 

此时p1 和 p2都是二级指针

![image-20251203204947203](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20251203204947203.png)

当object销毁之后,proxy对象继续存在,即便其值变为0 ,而p2也没有变成空悬指针.它可以通过proxy的内容来判断obj是否存货

![image-20251203205322897](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20251203205322897.png)

那么, 何时释放proxy指针呢?

## 一个更好的解决办法

引入引用计数,再把p1 和 p2指针 变成 对象sp1 和 sp2. 

proxy 现在有两个成员: 指针 和 计数器

1. 有两个引用 计数值为2

![image-20251203211851268](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20251203211851268.png)

2. sq1析构,引用计数的值减为一

   ![image-20251203212056745](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20251203212056745.png)

   1. sq2也析构了 ,引用计数降为0,可以安全的销毁proxy和object了

   ![image-20251203212238603](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20251203212238603.png)

这就是**引用计数型指针**的原理

## 万能的解决方案

引入另外一层间接性(another layer of indirection) 用对象来管理共享资源(如果把Object看作资源的话),亦即handle/body的管用技法(idiom)



# shared_ptr/weak_ptr

shared_ptr 是引用计数型智能指针, 引用计数是自动化资源管理的常用手法,当引用计数降为0时,对象(资源)即被销毁

weak_ptr 也是一个引用计数型智能指针,但是它不增加引用的次数,即弱(weak)引用.

- shared_ptr 控制对象的生命期, shared_ptr是强引用 
	- 只要有一个指向x对象的shraed_ptr存在 该x对象就不会析构
	- 当指向对象x的最后一个shared_ptr析构或reset()的时候,x保证会被销毁
- 
- weak_ptr 不控制对象生命期,但是它可以知道对象是否存活,
	- 如果对象存活,那么它可以提升(promote)为有效的`shared_ptr` 
	- 如果对象已经死了 提升会失败,返回一个空的`shared_ptr` 提升 lock() 行为是线程安全的
- shared_ptr / weak_ptr 的"计数" 在主流平台都是原子操作,没有用锁
- shared_ptr / weak_ptr 的线程安全级别与std::string 和 容器一样


# 系统的避免各种指针错误

c++ 里可能出现的内存错误大概有以下几个方面

1. 缓冲区溢出 ( buffer overun )
2. 空悬指针/野指针
3. 重复释放 ( double delete )
4. 内存泄漏 ( memory leak )
5. 不配对的 new\[ \] / delete
6. 内存碎片 ( memory fragmentation )

正确使用智能指针很容易解决前五个问题, 

[TODO]

1. 缓冲区溢出: 
	1. 使用`std::vector<char> / std::string ` 或自己编写 Buffer class 来管理缓冲区, 自动记住缓冲区长度,并通过成员函数而不是raw pointer 来修改缓冲区
2. 空悬指针/野指针 : 用`shared_ptr/ weak_ptr` 
3. 重复释放: 用`scoped_ptr / std::unique_ptr` ,只在对象析构时释放一次
4. 内存泄漏: 用`scoped_ptr / std::unique_ptr`. 对象析构的时候自动释放内存
	1. 不配对的new\[ \] / delete : 把 new\[ \] 替换为 `std::vector / scoped_array 

> 需要注意一点：
>
> scoped_ptr/shared_ptr/weak_ptr都是值语意，要么是栈上对
> 象，或是其他对象的直接数据成员，或是标准库容器里的元素。几乎不会有下面这种
> 用法：
>
> ```c++
> shared_ptr<Foo>* pFoo = new shared_ptr<Foo>(new Foo); // wRONG semantic
> ```
>
> 还要注意，如果这几种智能指针是对象x的数据成员，而它的模板参数T是个
> incomplete类型，那么x的析构函数不能是默认的或内联的，必须在.cpp文件里边
> 显式定义，否则会有编译错或运行错


#  应用到Observer 上

既然通过weak_ptr能探查对象的生死,那么Observer模式的竞态条件就很容易解决

> 只要让Observerable保存 `weak_ptr<Observer>` 即可

```cpp


class Observable
{
 public:
  void register_(boost::weak_ptr<Observer> x);
  // void unregister(boost::weak_ptr<Observer> x);
  void notifyObservers();
 private:
  mutable muduo::MutexLock mutex_;
  std::vector<boost::weak_ptr<Observer> > observers_;
  typedef std::vector<boost::weak_ptr<Observer> >::iterator Iterator;
};

void Observerable::notifyObservers() 
{
    muduo::MutexLockGuard lock(mutex_);
    Iterator it = observers_.begin();
    while (it != observers_.end()) {
      boost::shared_ptr<Observer> obj(it->lock());
      if (obj) {
        obj->update();
        ++it;
      }
      else {
        printf("notifyObservers() erase\n");
        it = observers_.erase(it);
      }
    }
}

```