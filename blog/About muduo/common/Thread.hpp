#ifndef __THREAD_HPP__
#define __THREAD_HPP__

#include "NoCopyAble.hpp"
#include <functional>
#include <memory>
#include <atomic>
#include <assert.h>

#if defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#else
static_assert(sizeof(void*) == 0, "Only Linux/macOS is supported");
#endif // linux/macOS

__thread pid_t t_cachedTid = 0;  // 线程局部存储变量,用于缓存当前线程的tid,避免每次调用CurrentThread::tid()时都进行系统调用获取tid

namespace CurrentThread
{
    __thread const char* t_threadName = "unnamedThread";
    pid_t tid() {
        if (t_cachedTid == 0)
        {
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));  // 获取当前线程的tid,并缓存到t_cachedTid中
        }
        return t_cachedTid;  // 返回当前线程的tid
    }
    const char* name() {
        return t_threadName;  // 返回当前线程的名称
    }
    bool isMainThread() // 判断当前线程是否为主线程
    {
        return tid() == ::getpid();  // 主线程的tid等于进程ID
    }
}

typedef struct _ThreadData {
    typedef std::function<void ()> ThreadFunc;
    ThreadFunc    func_;
    std::string   name_;
    std::weak_ptr<pid_t> wktid_;  // 使用weak_ptr避免循环引用,线程结束后tid_仍然有效,避免悬空指针

    _ThreadData(
        const ThreadFunc& func, 
        const std::string& name,
        std::shared_ptr<pid_t> wktid
    ) : func_(func), name_(name), wktid_(wktid) {}

    void runInThread() {
        pid_t tid = CurrentThread::tid();  // 获取当前线程的tid
        std::shared_ptr<pid_t> ptid = wktid_.lock();  // 获取tid的shared_ptr
        if (ptid) {
            *ptid = tid;  // 将当前线程的tid写入shared_ptr中,供Thread对象使用
            ptid.reset();  // 重置shared_ptr,释放对tid的所有权,避免循环引用
        }
        
        if (!name_.empty()) {
            CurrentThread::t_threadName = name_.c_str();  // 设置当前线程的名称,供调试和日志输出使用
        } else {
            CurrentThread::t_threadName = "muduoThread";  // 如果没有指定线程名称,使用默认名称
        }

        func_();  // 执行线程函数
        CurrentThread::t_threadName = "finished";  // 设置当前线程的名称为"finished"
    }
}ThreadData,*PThreadData;

class Thread : NoCopyAble
{
public:
    typedef std::function<void ()> ThreadFunc;
    explicit Thread(const ThreadFunc&, const std::string& name = std::string());
    ~Thread();
    void start();
    int join();
    bool started() const { return started_; }
    pid_t tid() const { return *tid_; }
    const std::string& name() const { return name_; }
    static int numCreated() { return numCreated_; }
private:
    bool                          started_;  ///< 是否已经启动线程
    bool                           joined_;  ///< 是否已经join线程
    std::shared_ptr<pid_t>            tid_;  ///< 线程ID,使用shared_ptr是为了在线程结束后tid_仍然有效,避免悬空指针
    pthread_t                   pthreadId_;  ///< pthread的线程ID,用于pthread_join等操作
    ThreadFunc                       func_;  ///< 线程函数对象,存储用户传入的线程函数
    std::string                      name_;  ///< 线程名称,用于调试和日志输出
    static std::atomic_int32_t numCreated_;  ///< 线程创建数量,使用原子类型保证线程安全,用于统计和调试
};

Thread::Thread(const ThreadFunc& func, const std::string& name)
    : started_(false),
      joined_(false),
      tid_(new pid_t(0)),  // 初始化tid_为0,表示线程未创建
      pthreadId_(0),       // 初始化pthreadId_为0,表示线程未创建
      func_(func),         // 存储用户传入的线程函数对象
      name_(name)          // 存储线程名称
{
    ++numCreated_;       // 线程创建数量加1
}

Thread::~Thread()
{
    if (started_ && !joined_)
    {
        pthread_detach(pthreadId_);  // 如果线程已经启动但未join,则分离线程,避免资源泄漏
    }
}

void* startThread(void* obj)
{
  ThreadData* data = static_cast<ThreadData*>(obj);
  data->runInThread();
  delete data;
  return NULL;
}

void Thread::start()
{
    started_ = true;  // 标记线程已经启动
    // 创建线程,传入线程函数和线程对象指针
    if (pthread_create(&pthreadId_, NULL, &startThread, this) != 0)
    {
        started_ = false;  // 如果创建线程失败,重置started_标志
        throw std::runtime_error("Failed to create thread");
    }
}

#endif // __THREAD_HPP__