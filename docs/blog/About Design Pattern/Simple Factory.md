# 简单工厂模式

## 简单工厂模式的应用场景

​	当我们的系统中存在大量同一类型的对象需要创建时，我们只需要传入一个正确的参数，就可以获取所需要的对象，而无需知道其实现过程。



## 简单工厂模式的实现要点

-  是由一个工厂类根据传入的参数，动态决定应该创建哪一个产品类

-  所有产品类均继承同一个父类

-  调用者不关心创建产品的细节，只关心产品的品牌

## 简单工厂模式的实现要点

其由以下三部分组成

- 工厂类（Factory）角色：

​		简单工厂模式的核心，它负责实现创建所有实例的内部逻辑。工厂类提供方法(或静态方法)，可以根据传入的参数，创建所需的产品对象。

- 抽象产品（Product）角色：

​		简单工厂模式所创建的所有对象的父类，它负责描述所有实例所共有的公共接口。可以是抽象类。

- 具体产品（Concrete Product）角色：

  ​	是简单工厂模式的创建目标，所有创建的对象都是充当这个角色的某个具体类的实例。

 ![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/9d21ccf48b2e439a8d7e7029ddf16628.png)


## 实现代码

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

class Factory
{
public:
    static Product* GetProduct(string arg)
    {
        Product* product = nullptr;
        if (arg == "A")
        {
            product = new ConcreteProductA();
            //init
        }
        else if (arg == "B")
        {
            product = new ConcreteProductB();
            //init
        }
        else
        {
            //....其他情况
        }

        return product;
    }
};



int main()
{
    string type;
        Product *product = nullptr;
        cout << "请输入要创建的产品类型：" << endl;
        cin >> type;
        product = Factory::GetProduct(type);//工厂类创建对象
        product->MethodName();
        product->work();
}

~~~

## 简单工厂模式的优点

-  工厂类包含必要的逻辑判断，可以决定在什么时候创建哪一个产品的实例。客户端可以免除直接创建产品对象的职责，很方便的创建出相应的产品。工厂和产品的职责区分明确。

-  客户端无需知道所创建具体产品的类名，只需知道参数即可。

-  也可以引入配置文件，在不修改客户端代码的情况下更换和添加新的具体产品类。

## 简单工厂模式的缺点

- 简单工厂模式的工厂类单一，负责所有产品的创建，职责过重，一旦异常，整个系统将受影响。且工厂类代码会非常臃肿，违背高聚合原则。

- 使用简单工厂模式会增加系统中类的个数（引入新的工厂类），增加系统的复杂度和理解难度

- 系统扩展困难，一旦增加新产品不得不修改工厂逻辑，在产品类型较多时，可能造成逻辑过于复杂

- 简单工厂模式使用了 static 工厂方法，造成工厂角色无法形成基于继承的等级结构。

 