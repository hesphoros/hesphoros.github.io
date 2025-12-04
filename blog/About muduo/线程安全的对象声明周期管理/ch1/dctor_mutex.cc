#include "MutexLockGuard.hpp"

class Foo{
public:
    Foo() { /* constructor implementation */ }
    ~Foo();
    void update();
private:
    MutexLock m_mtx;
};

Foo::~Foo() {
	MutexLockGuard lock(m_mtx);
	//(1) 
} 

void Foo::update() {
	MutexLockGuard lock(m_mtx);
	// (2)
}

extern Foo* x; // visiable by all threads

//thread A
void threadA() {
    delete x;
    x = nullptr;
}

// thread B
void threadB() {
    if(x) {
        x->update();
    }
}


//尽管 threadA 在销毁对象之后把指针设为 nullptr
//尽管 threadB 在调用x->update()之前检查了x是否为nullptr
//但仍然存在竞态
//可能的执行顺序如下：
//1. 线程A执行到了析构函数的 MutexLockGuard lock(m_mtx); 处，成功持有锁 即将继续往下执行
//2. 线程B通过了 if(x) 检查,阻塞在了 MutexLockGuard lock(m_mtx); 处，等待锁
//3. 线程A继续执行，完成析构函数，释放锁
//4. 线程B获得锁，继续执行x->update()，此时x已经是一个悬空指针，导致未定义行为

//解决方法：使用一个全局的MutexLock保护对x的访问
MutexLock g_mtx; // global mutex to protect access to x
// thread A
void threadA_fixed() {
    MutexLockGuard lock(g_mtx); // Lock the global mutex
    delete x;
    x = nullptr;
}

// thread B
void threadB_fixed() {
    MutexLockGuard lock(g_mtx); // Lock the global mutex
    if(x) {
        x->update();
    }
}
