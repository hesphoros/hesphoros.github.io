# 单例模式

## 应用场景

​	用来创建唯一的实例对象并保证对象唯一

## 单例模式实现要点

- 构造函数私有化 - 为避免其他程序过多建立该类对象，先禁止其他程序建立该类对象

- 类中创建一个本类对象 - 本类中自定义对象，让其他程序可以访问

- 提供方法获取到该对象 - 方便其他程序对自定义对象的访问

## 实现方式

### 懒汉式(线程不安全)

懒汉式（多线程不安全）。需要时候才进行实例化

~~~cpp
#include <iostream>
 

class Singleton{
    private:
        Singleton(){}
        Singleton(Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
    public:
        ~Singleton(){}
        static Singleton* getInstance(){
            if(m_instance_ptr == nullptr){
                m_instance_ptr = new Singleton();
            }
            return m_instance_ptr;
        }     
        void use() const { std::cout << "in use" << std::endl; } 
    private:
        static Singleton* m_instance_ptr ;
};


Singleton* Singleton::m_instance_ptr = nullptr;

int main() {
	Singleton* instance = Singleton::getInstance();
	instance->use();

	system("pause");
	return 0;
}
~~~



1. 线程安全的问题：第一个线程在if中判断 `m_instance_ptr`是空的，于是开始实例化单例;同时第2个线程也尝试获取单例，这个时候判断`m_instance_ptr`还是空的，于是也开始实例化单例;这样就会实例化出两个对象,这就是线程安全问题的由来; 可以使用加锁解决
2. 可能存在内存泄漏`m_instance_ptr`,并不会被释放，除非显示调用

### 懒汉式（线程安全）

线程安全 + 自动内存释放（使用shared_ptr）

~~~cpp
#include <iostream>
#include <memory> // shared_ptr
#include <mutex>  // mutex

class Singleton {
public:
	typedef std::shared_ptr<Singleton> Ptr;
	~Singleton() {
		std::cout << "Singleton - destructor." << std::endl;
	}

	Singleton(Singleton&) = delete; //禁止使用
	Singleton& operator=(const Singleton&) = delete;  //禁止使用

	static Ptr getInstance() {

		if (m_instance_ptr == nullptr) {
			std::lock_guard<std::mutex> auto_lock(m_mutex);
			if (m_instance_ptr == nullptr) {
				 
                m_instance_ptr = std::make_shared<Singleton>();
                /*错误处理：使用new创建std::shared_ptr时，如果构造函数抛出异常，
                那么智能指针将无法正确地释放内存。使用std::make_shared时，如果构造函数抛出异常，
                std::make_shared将自动释放已经分配的内存。*/
			}
		}
		return m_instance_ptr;
	}

	void use() const { 
		std::cout << "Singleton - in use." << std::endl; 
	}


private:
	Singleton() {}
	static Ptr m_instance_ptr;
	static std::mutex m_mutex;
};
Singleton::Ptr Singleton::m_instance_ptr = nullptr;
std::mutex Singleton::m_mutex;

int main() {
	Singleton::Ptr instance = Singleton::getInstance();
	instance->use();

	system("pause");
	return 0;
}
~~~

`getInstance`函数返回一个`std::shared_ptr<Singleton>`类型的智能指针。当智能指针被销毁时，`Singleton`对象也会被自动销毁，这样可以确保只有一个`Singleton`对象被创建，并且对象的生命周期得到自动管理。

### 饿汉式



- ，在单例定义的时候进行实例化

  ~~~cpp
  #include <iostream>
  
  class Singleton {
  public:
  
  	~Singleton() {
  		std::cout << "Singleton - destructor." << std::endl;
  	}
  
  	Singleton(Singleton&) = delete; //禁止使用
  	Singleton& operator=(const Singleton&) = delete;  //禁止使用
  
  	static Singleton* getInstance() {
  		return m_instance;
  	}
  
  	void use() const { 
  		std::cout << "Singleton - in use." << std::endl; 
  	}
  
  
  private:
  	Singleton() {}
  	static Singleton * m_instance;
  
  };
  
  Singleton* Singleton::m_instance = new Singleton();
  
  
  int main() {
  	Singleton* instance = Singleton::getInstance();
  	instance->use();
  
  	system("pause");
  	return 0;
  }
  ~~~

  