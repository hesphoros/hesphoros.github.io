#include "../../common/MutexLock.hpp"
#include "../../common/MutexLockGuard.hpp"
#include "../../common/Thread.hpp"
#include <vector>
#include <stdio.h>

class Foo
{
 public:
  void doit() const;
  void error1() const;
};


/// post() 加锁修改了共享资源 foos, traverse() 加锁遍历了共享资源 foos
/// 这都是没问题的。但是如果如下操作：
/// Foo::doit() 中调用 post()，就会死锁。
/// 因为 post() 和 traverse() 都需要获取 mutex 锁，
/// 而 post() 在调用 traverse() 时已经持有了 mutex 锁，所以 traverse() 无法获取到 mutex锁而导致死锁

/// 1. mutex 是非递归锁，不能被同一线程多次加锁。
/// 2. mutex 是递归锁 （如 pthread_mutex_t 的 PTHREAD_MUTEX_RECURSIVE 类型
///   由于push_back() 可能会导致 vector 重新分配内存，导致迭代器失效程序崩溃，所以 post() 和 traverse() 不能同时持有 mutex 锁。

MutexLock mutex;
std::vector<Foo> foos;


void post(const Foo& f)
{
  MutexLockGuard lock(mutex);
  foos.push_back(f);
}

void traverse()  // 线程安全的遍历foos
{
  MutexLockGuard lock(mutex);
  for (std::vector<Foo>::const_iterator it = foos.begin();
      it != foos.end(); ++it)
  {
    it->doit();
  }
}

void Foo::doit() const
{
  Foo f;
  post(f);
}

int main()
{
  Foo f;
  post(f);
  traverse();
}


/// post 加锁和不加锁函数分离

void postWithLockHold(const Foo& f) /// 此函数调用者必须持有 mutex 锁
{
  foos.push_back(f);
}

void post(const Foo& f)  /// 此函数负责加锁，调用 postWithLockHold() 来修改共享资源
{
  MutexLockGuard lock(mutex);
  postWithLockHold(f);
}


/// 然而这样可能导致两个问题
/// a.误用了加锁版本死锁了 （锁的语义不明确
/// 例如：
void Foo::error1() const
{
    MutexLockGuard lock(mutex);  // 已经持锁
    Foo f;
    post(f);                     // post 内部还会再加锁 → 死锁
}

void someFunc()
{
    MutexLockGuard lock(mutex);

    Foo f;
    postWithLockHold(f);  //  postWithLockHold() 不加锁，调用者必须持有锁

    traverse();           //  traverse 也会加锁 → 死锁
}

/// b.误用了不加锁版本，数据损坏了UB （没有强制同步机制

void workerThread()
{
    Foo f;
    postWithLockHold(f);  //  忘记加锁
}
/// 线程 A 调用 traverse()，线程 B 调用 postWithLockHold()，同时访问共享资源 foos，导致数据损坏和未定义行为。
/// vector 扩容场景如下：
/// foos.push_back(f); // 触发 reallocate 操作，导致迭代器失效
/// traverse() 中的迭代器 it 可能指向已经被释放的内存地址，访问 it->doit() 就会导致程序崩溃。
/// resize被并发修改场景如下:
/// 线程A正在读size（） 线程B在写
/// 结果结果：读到脏数据 越界访问

/// b 的解决方案

void postWithLockHold(const Foo& f) /// 此函数调用者必须持有 mutex 锁
{
  assert(mutex.isLockedByThisThread()); // 断言调用者必须持有锁
  foos.push_back(f);
}