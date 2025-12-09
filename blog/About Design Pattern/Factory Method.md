# 工厂方法模式

## 工厂方法模式应用场景

 专门的工厂实现专门一类的产品。

能解决简单工厂的创建方法中增加产品就要增加新的if-else的问题，解决这种做法带来的扩展性差，违背开闭原则，和影响到可读性等问题。

## 工厂方法模式实现要点

工厂方法模式由抽象工厂、具体工厂、抽象产品和具体产品等4个要素构成。其基本结构如下。

- 抽象工厂（Abstract Factory）：提供了创建产品的接口，调用者通过它访问具体工厂的工厂方法` newProduct()` 来创建产品。

- 具体工厂（Concrete Factory）：主要是实现抽象工厂中的抽象方法，完成具体产品的创建。

- 抽象产品（Product）：定义了产品的规范，描述了产品的主要特性和功能。
- 具体产品（Concrete Product）：实现了抽象产品角色所定义的接口，由具体工厂来创建，它同具体工厂之间一一对应。

![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/f53136b86b724cfba79550df01c8cbee.png)


## 代码实现

~~~cpp
#include <iostream>
#include <string>

using namespace std;

class Product
{
public:
    virtual void MethodName()
    {
        //公共方法的实现
    }
    virtual void work() = 0;
    //声明抽象业务方法
};

class ConcreteProductA : public Product
{
public:
    void MethodName()
    {
        cout << "我是海尔牌-洗衣机." << endl;
    }

    void work()
    {
        //业务方法的实现
        cout << "我是小海洗，我能洗刷刷..." << endl;
    }
};

class ConcreteProductB : public Product
{
public:
    void MethodName()
    {
        cout << "我是海尔牌-空调." << endl;
    }
    
    void work()
    {
        //业务方法的实现
        cout << "我是小海空，我让你凉爽爽..." << endl;
    }
};

class FactoryBase
{
public:
    FactoryBase() {}
    ~FactoryBase() {}

    virtual Product* GetProduct() = 0;
};


class AFactory: public FactoryBase
{
public:
    AFactory() {}
    ~AFactory() {}

    Product* GetProduct()
    {
        return new ConcreteProductA();
    }
};

class BFactory : public FactoryBase
{
public:
    BFactory() {}
    ~BFactory() {}

    Product* GetProduct()
    {
        return new ConcreteProductB();
    }
};



int main()
{
	string type;
	Product* productA = nullptr;
    Product* productB = nullptr;
	cout << "请输入要创建的产品类型：" << endl;

    FactoryBase* factoryA = new AFactory();
    FactoryBase* factoryB = new BFactory();
	
	productA = factoryA->GetProduct();//工厂A类创建对象
	productA->MethodName();
	productA->work();

    productB = factoryB->GetProduct();//工厂B类创建对象
    productB->MethodName();
    productB->work();
}

~~~

## 工厂方法模式的缺点

- 每增加一种产品，就需要增加一个对象的工厂。如果这家公司发展迅速，推出了很多新的产品，那么每一个产品就要开设相应的新工厂。那么，我们在C++实现中，就要定义与产品同等数量的工厂类。显然，相比简单工厂模式，工厂方法模式需要更多的类定义。这样会使得系统中类的个数成倍增加，增加了代码的复杂度。