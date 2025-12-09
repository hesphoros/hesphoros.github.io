# 适配器模式

## 适配器模式应用场景

​	Rock老师买了一个日本进口机器人，机器人充电要求电压（110V）与国内插头标准电压（220V）不兼容，咋办？

​	解决方案：

> ​	设置一个适配器将插头输出的220V转变为110V

以上的解决方案就是通过一个适配器来解决接口不一致的问题，使原本由于接口不兼容而不能一起工作的那些类可以在一起工作



## 适配器模式的实现要点

- 定义一个包装类，用于包装不兼容接口的对象
- 包装类 =  适配器Adapter

- 类的适配器模式是把适配器的类的API转换为成为目标类的API
- 

## 设计类图

![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/b4f7f2cdf6284f3fb5eb7c34c87094a1.png)


对象适配器模式的结构图如下图所示

~~~cpp
#include <iostream>
#include <string>

//类模式
namespace adapter_class{
    //目标接口
    class Target{
    public:
        virtual void request() = 0;
    };

    //适配者类
    class Adaptee{
    public:
        void specificRequest(){
            std::cout<< "适配者中的业务代码被调用~"<<std::endl;

        }
    };
    //类适配器类
    class ClassAdapter: public Target,private Adaptee{
    public:
        ClassAdapter(){};
        ~ClassAdapter(){}; 
        virtual void request(){
            std::cout<<"Adapter::Request()"<<std::endl;
            this->specificRequest();
            std::cout<<"--------------------"<<std::cout;
        }
    };

};

//对象模式
namespace adapter_obj{
    class Target{
        public:
            virtual void request() = 0;
    };
    class Adaptee{
    public:
        void specificRequest(){
            std::cout<< "适配者中的业务代码被调用~"<<std::endl;

        }
    };
    class Adapter1:public Target{
        public:
            Adapter1(Adaptee* adaptee):_adaptee(adaptee){};
            ~Adapter1() {};
            virtual void request(){_adaptee->specificRequest();}

        private:
            Adaptee* _adaptee;
    };
};

int main(void){
    std::cout<<"类适配器模式测试："<<std::endl;
    adapter_class::Target *target = new adapter_class::ClassAdapter();
    target->request();

    std::cout<<"对象适配器模式测试："<<std::endl;
    adapter_obj::Adaptee *ade       =     new adapter_obj::Adaptee();
    adapter_obj::Target* pTarget    =     new adapter_obj::Adapter1(ade);
    

}
~~~

## 适配器模式的优点

