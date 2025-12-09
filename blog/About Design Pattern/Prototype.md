

# 原型模式

## 原型模式使用场景

 

​	*当在我们的系统中存在大量相同或相似对象的创建问题，如果用传统的构造函数来创建对象，会比较复杂且耗时耗资源（主要是构造函数的执行），用原型模式生成对象就很高效，直接内存式的拷贝。*



## 原型模式实现要点

- 用一个已经创建的实例对象作为原型
- 通过复制该原型对象来创建一个和原型对象相同或者相似的新对象

## 原型对象的实现方式

### 原型模式的结构

1. 抽象原型类：规定了具体原型对象必须实现的接口。
2. 具体原型类：实现抽象原型类的 `clone()` 方法，它是可被复制的对象。
3. 访问类：使用具体原型类中的` clone() `方法来复制新的对象。

![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/d10f1fdbeaa24940954d0ab9b619066f.png)


~~~cpp
#include <cstdio>
#include <iostream>

using namespace std;


class Prototype {
protected:
    string prototype_name_;
    float prototype_field_;

public:
    Prototype() {}
    Prototype(string prototype_name)
        : prototype_name_(prototype_name) {
    }
    virtual ~Prototype() {}

    virtual Prototype* Clone() const = 0;

    virtual void Method(float prototype_field) {
        this->prototype_field_ = prototype_field;
        std::cout << "Call Method from " << prototype_name_ << " with field : " << prototype_field << std::endl;
    }
};

class Realizetype1 : public Prototype {
private:
    float realize_prototype_field1_;

public:
    Realizetype1(string prototype_name, float concrete_prototype_field)
        : Prototype(prototype_name), realize_prototype_field1_(concrete_prototype_field) {
    }

    /**
     * 注意，Clone 方法返回一个新的克隆对象的指针，调用者必须释放其内存。
     */
    Prototype* Clone() const override {
        return new Realizetype1(*this); //浅拷贝
    }
};


class Realizetype2 : public Prototype {
private:
    float realize_field2_;

public:
    Realizetype2(string prototype_name, float concrete_prototype_field)
        : Prototype(prototype_name), realize_field2_(concrete_prototype_field) {
    }
    Prototype* Clone() const override {
        return new Realizetype2(*this);
    }
};



int main()
{
    Realizetype1  sxz("孙悟空", 1000);
    Realizetype2  zbj("猪八戒", 800);

    Realizetype1* s1 = (Realizetype1*)sxz.Clone();
    Realizetype2* z2 = (Realizetype2*)zbj.Clone();

    s1->Method(1000);
    z2->Method(800);

    return 0;
}
~~~

## 原型模式的实现优点

-  创建对象基于内存二进制流的复制，在性能上比直接 new 一个对象更加优良。

-  可以使用深克隆方式保存对象的状态，使用原型模式将对象复制一份，并将其状态保存起来，简化了创建对象的过程，以便在需要的时候使用（例如恢复到历史某一状态），可辅助实现撤销操作。



## 原型模式的实现的缺点

-  需要为每一个类都配置一个 clone 方法

-  clone 方法位于类的内部，当对已有类进行改造的时候，需要修改代码。

-  当实现深克隆时，需要编写较为复杂的代码，而且当对象之间存在多重嵌套引用时，为了实现深克隆，每一层对象对应的类都必须支持深克隆，实现起来会比较麻烦。因此，深克隆、浅克隆需要运用得当。