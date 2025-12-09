# 建造者模式

## 应用场景

​	当一个类的构造函数参数有多个时，比如超过四个，而且这些参数有些是可选时，我们通常有两种方法来构建它的对象。例如我们现在有一个如下类`Computer`，其中`CPU`和`ram`是必填参数，而其他3个是可选参数，那么我们如何构造这个类的实例呢，通常有两种常用方式：

~~~c
#include <iostream>
#include <string>

class Computer1{
private:
    //必选
    std::string cpu;
    std::string ram;
    
    //可选
    std::string keyboard;
    std::string display;
public:
    Computer1();
    Computer1(std::string cpu,std::string ram){
        Computer1(cpu,ram,"罗技");
    }
    
    Computer1(std::string cpu,std::string ram,std::string keyboard){
        Computer1(cpu,ram,"三星显示器");
    }
    Computer1(std::string cpu,std::string ram,std::string keyboard,std::string display)
        :cpu(cpu),ram(ram),keyboard(keyboard),display(display){}

};

class Computer2{
private:
    //必选
    std::string cpu;
    std::string ram;
    
    //可选
    std::string keyboard;
    std::string display;
public:
    Computer2(std::string cpu,std::string ram):cpu(cpu),ram(ram){}
    void setCpu(std::string name){this->cpu = name;}
    void SetRam(std::string name){this->ram = name;}
    void setKeyboard(std::string name){this->keyboard = name;}
    void setDisplay(std::string name){this->display = name;}
};


 
~~~

​	那么这两种方式有什么弊端呢？

1. 第一种方式主要是使用及阅读不方便，参数较多，调用者分不清参数的具体含义。同时，参数太多容易混淆
2. 第二种方式在构建过程中对象的属性状态因为类中的属性是分布设置的，容易发生变化，造成错误

## 建造者模式的实现要点

- 在`Computer`中创建一个静态的`Builder`,负责产品`Computer`对象的创建
- 创建一个指挥者`Director`，指挥不同的`Builder`对象创建不同`Computer`对象
- 在`Builder`中创建一个`public`的构造函数，参数为`Computer`中必填的参数，本例子中的`CPU`和`ram`
- 在`Builder`中创建设置函数，对`computer`中那些可选的参数进行赋值
- 在`Builder`中创建一个`build()`方法，在其中构建`Computer`的实例并返回



## 建造者模式的实现方式

建造者模式由指挥者、抽象建造者、具体建造者和产品等4个要素构成，其基本结构如下。



指挥者角色（`Director`) ：

- ​	构建一个是由`Builder`接口创建的对象

抽象建造则模式（`Builder`）：

- ​	为创建一个`Product`对象的各个部件指定的抽象接口

具体建造者模式 （`ConcreteBuilder`): 

- ​	实现`Builder`接口，构造和装配多个部件

产品角色（`Product`）：

- ​	待生成的具体产品

## 设计类图

 ![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/84e82d406183466ba024ca872729e08a.png)

## 代码实现

~~~cpp
#include <iostream>
#include <string>

using namespace std;

class Product{


private:
        string partA;//必选参数
        string partB;
        string partC;
public:
    Product(std::string partA):partA(partA){}
    //省略get   以及toString方法
    void setPartA(string param){this->partA = param;}
    void setPartB(string param){this->partB = param;}   
    void setPartC(string param){this->partC = param;}      

    void show(){
        cout<<"Product[partA="<<partA<<",partB = "<<partB<<",partC =  "<<partC<<"]";
        
    }
};

//虚基类
class Builder{
    public:
        virtual void        setPartA() = 0;
        virtual void        setPartB() = 0;
        virtual void        setPartC() = 0;
        
        virtual Product*    build() = 0;
};

class ConcreteBuilder: public Builder{
private:
    Product * product;
public: 
    ConcreteBuilder(std::string partA){
        product = new Product(partA);
    }

    void setPartA(){product->setPartA("A");}
    void setPartB(){product->setPartB("B");}
    void setPartC(){product->setPartC("C");}
    Product* build(){return product;}
};

class Director{

private:
    Builder * builder;
public:
    Director(Builder* builder){//传入build子类对象
        this->builder = builder;
    }

    Product* constructA(){
        
        builder->setPartB();
        builder->setPartC();
        return builder->build();
    }

      Product* constructB(){
        builder->setPartA();    
        builder->setPartC();
        return builder->build();
    }
};

void BuildTest(){
    Builder * builder  = new ConcreteBuilder("partA");
    Director* director = new Director(builder);
    
    Product * productA = director->constructA();
    Product * productB = director->constructB();
    
    productA->show(),productB->show();
     
}

int main()
{
    BuildTest();
    return 0;
}
~~~

## 建造者模式的优点

- 封装性： 是客户端不必知道产品内部组成的细节
- 建造者独立，易扩展
- 便于控制细节风险，可以对建造过程逐步细化，而不对其他模块产生任何影响

## 建造者模式的缺点 

- **增加类数量 :** 产生多余的 Builder 对象 ;
- **内部修改困难 :** 如果 产品内部发生变化 , 建造者也要相应修改 ;

