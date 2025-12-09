# 抽象工厂模式

## 抽象工厂模式应用场景

​	随着科技革命和智能智造的运用，海尔的工厂也运用了先进的智能设备进行生产，运用智能智造技术后，很多同族产品就可以进行共线生产了，这样可以大大缩减工厂实现子的数量,比方说海尔冰箱和洗衣机，此时，随着海尔海外业务的增长，海尔在国内和新加坡都建立了这种共线工厂，用来专门生产其冰箱和洗衣机，这样，国内的电器采购商，可以向国内的工厂订购，国外的电器采购商，就需向国外的采购商采购！

和工厂方法不一样的是，抽象工厂接口生产同一产品家族中的多个产品（两个或以上），抽象工厂的每个子类（具体工厂）都负责生产这些产品。

抽象工厂模式适用于以下情况：

1. 一个系统要独立于它的产品的创建、组合和表示时；

2. 一个系统要由多个产品系列中的一个来配置时；

3. 要强调一系列相关的产品对象的设计以便进行联合使用时；

4. 当你提供一个产品类库，而只想显示它们的接口而不是实现时；

## 抽象工厂模式实现要点

工厂方法模式由抽象工厂、具体工厂、抽象产品和具体产品等4个要素构成。其基本结构如下。

- 抽象工厂（Abstract Factory）：提供了创建产品的接口，调用者通过它访问具体工厂的工厂方法 `newProduct()` 来创建产品。

- 具体工厂（Concrete Factory）：主要是实现抽象工厂中的抽象方法，完成具体产品的创建。

- 多个同族抽象产品（Product）：定义了产品的规范，描述了产品的主要特性和功能。

- 多个同族具体产品（Concrete Product）：它是具体产品继承的父类或者是实现的接口。

 
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/89063a74a9c946338423de65af506a4b.png)

## 实现源码

~~~cpp
#include <iostream>
#include <string>

using namespace std;

class ProductA//洗衣机
{
public:
    virtual void MethodName()
    {
        //公共方法的实现
    }
    virtual void work() = 0;
    //声明抽象业务方法
};

class ConcreteProductA1 : public ProductA
{
public:
    void MethodName()
    {
        cout << "我是海尔牌-国内洗衣机." << endl;
    }

    void work()
    {
        //业务方法的实现
        cout << "我是国内小海洗，我能洗刷刷..." << endl;
    }
};

class ConcreteProductA2 : public ProductA
{
public:
    void MethodName()
    {
        cout << "我是海尔牌-国外洗衣机." << endl;
    }
    
    void work()
    {
        //业务方法的实现
        cout << "我是国外小海空，我让你凉爽爽..." << endl;
    }
};

class ProductB//空调
{
public:
    virtual void MethodName()
    {
        //公共方法的实现
    }
    virtual void work() = 0;
    //声明抽象业务方法
};

class ConcreteProductB1 : public ProductB
{
public:
    void MethodName()
    {
        cout << "我是海尔牌-国内空调." << endl;
    }

    void work()
    {
        //业务方法的实现
        cout << "我是国内小海空，我让你凉爽爽..." << endl;
    }
};

class ConcreteProductB2 : public ProductB
{
public:
    void MethodName()
    {
        cout << "我是海尔牌-国外空调." << endl;
    }

    void work()
    {
        //业务方法的实现
        cout << "我是国外小海空，我让你凉爽爽..." << endl;
    }
};

class FactoryBase
{
public:
    FactoryBase() {}
    ~FactoryBase() {}

    virtual ProductA* GetProductA() = 0;
    virtual ProductB* GetProductB() = 0;
};


class HomeFactory: public FactoryBase
{
public:
    HomeFactory() {}
    ~HomeFactory() {}

    ProductA* GetProductA()
    {
        return new ConcreteProductA1();
    }

    ProductB* GetProductB()
    {
        return new ConcreteProductB1();
    }
};

class ForeignFactory : public FactoryBase
{
public:
    ForeignFactory() {}
    ~ForeignFactory() {}

    ProductA* GetProductA()
    {
        return new ConcreteProductA2();
    }

    ProductB* GetProductB()
    {
        return new ConcreteProductB2();
    }
};



int main()
{
    //国内冰洗
	ProductA* productA1 = nullptr;
    ProductB* productB1 = nullptr;

    //国外冰洗
    ProductA* productA2 = nullptr;
    ProductB* productB2 = nullptr;

    FactoryBase* factoryH = new HomeFactory();
    FactoryBase* factoryF = new ForeignFactory();
	
	productA1 = factoryH->GetProductA();//工厂A类创建对象
	productA1->MethodName();
	productA1->work();

    productB1 = factoryF->GetProductB();//工厂B类创建对象
    productB1->MethodName();
    productB1->work();
}

~~~

## 抽象工厂模式的优点

-  抽象工厂模式最大的好处便是易于交换产品系列，只要改变创建的工厂对象就可以实现产品系列的更换。还有一个优点是它让具体的创建实例的过程于客户端分离，客户端是通过它们抽象接口操纵实例，产品的具体类名也被具体工厂的实现分离，不会出现在客户代码中。最大的缺点是如果像添加产品则非常麻烦需要修改很多类。