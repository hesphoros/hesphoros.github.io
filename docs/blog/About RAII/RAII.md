# RAII
_资源获取即初始化（Resource Acquisition Is Initialization）_，简称 RAII，是一种 C++ 编程技术，它将必须在使用前获取的资源（已分配的堆内存、执行线程、打开的套接字、打开的文件、锁定的互斥量、磁盘空间、数据库连接——任何供应有限的资源）的生命周期与对象的[生命周期](https://cppreference.cn/w/cpp/language/lifetime "cpp/language/lifetime")绑定。

RAII 的核心思想就是: **利用栈上局部变量的自动析构来保证资源一定会被释放**。

RAII 可以总结如下：

- 将每个资源封装到一个类中，其中
	- 构造函数获取资源并建立所有类不变量，如果无法完成则抛出异常，
	- 析构函数释放资源，并且从不抛出异常；

- 始终通过 RAII 类的实例使用资源，该实例要么
	- 具有自动存储期或临时生命周期，要么
	- 其生命周期受自动或临时对象的生命周期限制。

# example1

## bad_do_file


```cpp
// Bad example: Not using RAII properly
// If an exception occurs during program execution 
// or the program exits prematurely, it may lead to 
// files not being closed, resulting in issues 
// such as resource leaks.
int bad_do_file() {
    std::ifstream myfile("example.txt");
    if (myfile.is_open()) {
        std::cout << "File is opened." << std::endl;
        // do something with the file...
    }
    else {
        std::cout << "Failed to open the file." << std::endl;
    }
    myfile.close();
    return 0;
}

```

## good_do_file
```cpp
// Good example: Using RAII properly
// 通过定义一个包含文件流的类，在类的构造函数中打开文件，
// 在析构函数中关闭文件：
class FileHandler {
public:
    FileHandler(const char* filename) : m_file_stream(filename) {
        if (!m_file_stream.is_open()) {
            throw std::runtime_error("Failed to open the file.");
        }
    }

    ~FileHandler() {
        if (m_file_stream.is_open()) {
            std::cout << "Closing the file." << std::endl;
            m_file_stream.close();
        }
    }

    std::ifstream& getStream() {
        return m_file_stream;
    }
private:
    std::ifstream m_file_stream;
};

int good_do_file() {
    try {
        FileHandler fileHandler("example.txt");
        std::ifstream& myfile = fileHandler.getStream();
        std::cout << "File is opened." << std::endl;
        // do something with the file...
    }
    catch (const std::runtime_error& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
```


# example_mutex
```cpp
#include <mutex>

std::mutex mtx;

void f() { /** do something */}
bool everything_ok() { /** check some condition */ }

//Bad example: Not using RAII properly
void bad_lock_example() {
    mtx.lock();     // acquire the lock
    f();            // If f() throw an exception, the mutex will not be released

    if ( !everything_ok())
        return;     // early return without releasing the lock
    mtx.unlock();   // release the lock
}

//Good example: Using RAII properly
void good_lock_example() {
	// RAII class: mutex acquisition is initialization
    std::lock_guard<std::mutex> lock(mtx); 
    
    // If f() throws an exception, the destructor of lock_guard will release
    f();                                  
    if ( !everything_ok())
	    // early return is safe, lock_guard will release the lock
        return;                            
}
```