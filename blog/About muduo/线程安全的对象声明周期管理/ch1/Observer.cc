#include <algorithm>
#include <vector>
#include <stdio.h>

class Observable;

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

  /**
   * (3) 在通知每一个观察者时 从何得知Observer对象o是否还存活? 
   * 一个动态创建的对象是否还活着,光看指针/引用是看不出来的
   * 指针就是指向了一块内存.如果这块内存上的对象已经销毁
   * 那么就不可能访问,既然不能访问如何又知道对象的状态呢? 
   * 
   * 判断一个指针是不是合法的指针没用高效的办法? 万一这个新的对象的类型异于老的对象呢?
   */
  void notifyObservers()
  {
    for (Observer* o : observers_)
    {
      if (o)
        o->update(); //( 3 )
    }
  }

 private:
  
  // 原始指针的核心问题: 它只是一个内存地址, 无法告诉你这个地址上的对象是否还存活
  std::vector<Observer*> observers_;

  // 使用Observer*的指针时:
  // 1. 你不知道它指向的对象是否已经被delete了
  // 2. 你不知道它指向的内存是否被重新分配给了其他对象
  // 3. 访问一个已经被delete的对象是未定义行为
  
};

// 两个race condition
// 1. 如何得知subject_还存活?
// 2. 就算subject_指向某个永久存在的对象 那么
//    2.1. ThreadA 执行到 subject_->unregister(this); 之前 ,还没有来得及unregester本对象
//    2.2. ThreadB 执行到 o->update ,o 正好指向是ThreadA正在析构的对象
// 此时,既然o指向的Observer对象正在析构,调用他的任何非静态成员函数都是不安全的 
// 何况是虚函数 ,而且Observer是个基类,执行 subject_->unregister(this); 的时候 
// 派生类已经析构了, 此时整个对象处于将死未死的状态,调用虚函数更是危险

// 派生类析构:
// 1. 先调用子类的析构函数
// 2. 再调用父类的析构函数
Observer::~Observer()
{
  subject_->unregister(this);  // 打扫战场：告诉被观察者"我要死了，别再通知我"
  // 避免Observable持有一个指向已经销毁的Observer对象的悬空指针
}

void Observer::observe(Observable* s)
{
  s->register_(this);
  subject_ = s;
}

void Observable::register_(Observer* x)
{
  observers_.push_back(x);
}

void Observable::unregister(Observer* x)
{
  std::vector<Observer*>::iterator it = std::find(observers_.begin(), observers_.end(), x);
  if (it != observers_.end())
  {
    std::swap(*it, observers_.back());
    observers_.pop_back();
  }
}

// ---------------------

class Foo : public Observer
{
  virtual void update()
  {
    printf("Foo::update() %p\n", this);
  }
};

int main()
{
  Foo* p = new Foo;
  Observable subject;
  p->observe(&subject);
  subject.notifyObservers();
  delete p;
  subject.notifyObservers();
}
