# Observer 观察者模式
## 核心思想

观察者模式的核心在于实现**松耦合**的分布式通知系统。它允许主题对象在不了解观察者具体类的情况下，通知所有相关的观察者。

## 角色构成

**`Subject` (主题/可观察者):**

- 它维护了一个**观察者列表**。
- 提供方法来**添加** (`Attach` / `Register`) 或**移除** (`Detach` / `Unregister`) 观察者。
- 提供方法来**通知** (`Notify`) 列表中的所有观察者，通常在自身状态发生变化时调用。

**`ConcreteSubject` (具体主题):**

- 存储观察者感兴趣的**核心状态**。
- 当状态改变时，会调用继承自 `Subject` 的 `Notify` 方法。

**`Observer` (观察者接口):**

- 为所有具体的观察者定义一个**更新接口** (`Update`)。当主题发出通知时，具体观察者需要通过实现此接口来更新自身。

**`ConcreteObserver` (具体观察者):**

- 实现 `Observer` 接口的 `Update` 方法，以响应主题的通知。
- 它们通常会保存一个指向**具体主题**的引用，以便在收到通知后查询主题的状态（**拉模型**）或接收主题传递过来的数据（**推模型**）。

## 示例代码

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

// --- 1. Observer 接口 ---
class Observer {
public:
    virtual void Update(const std::string& message) = 0;
    virtual ~Observer() = default;
};

// --- 2. Subject 抽象基类 ---
class Subject {
public:
    // 附加观察者
    void Attach(Observer* observer) {
        observers_.push_back(observer);
    }

    // 分离观察者
    void Detach(Observer* observer) {
        // 使用 erase-remove idiom 安全移除元素
        observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
    }

    // 通知所有观察者
    void Notify(const std::string& message) {
        for (Observer* obs : observers_) {
            obs->Update(message);
        }
    }

    virtual ~Subject() = default;

private:
    std::vector<Observer*> observers_;
};

// --- 3. ConcreteSubject 具体主题 ---
class NewsPublisher : public Subject {
public:
    void PublishNews(const std::string& news) {
        std::cout << "\n[Publisher] New news released: " << news << std::endl;
        // 状态变化，通知所有观察者
        Notify(news);
    }
};

// --- 4. ConcreteObserver 具体观察者 ---
class Subscriber : public Observer {
public:
    Subscriber(const std::string& name) : name_(name) {}

    void Update(const std::string& message) override {
        std::cout << "[" << name_ << "] received update: " << message << std::endl;
    }

private:
    std::string name_;
};

/*
int main() {
    // 创建主题
    NewsPublisher publisher;

    // 创建观察者
    Subscriber sub1("Alice");
    Subscriber sub2("Bob");
    Subscriber sub3("Charlie");

    // 观察者订阅主题
    publisher.Attach(&sub1);
    publisher.Attach(&sub2);

    // 主题发布新闻，通知 Alice 和 Bob
    publisher.PublishNews("Tech stocks soar!");

    // Charlie 订阅
    publisher.Attach(&sub3);

    // Bob 取消订阅
    publisher.Detach(&sub2);

    // 主题发布第二条新闻，只通知 Alice 和 Charlie
    publisher.PublishNews("Market correction imminent.");

    return 0;
}
*/
```

## 优势

- **松耦合 (Loose Coupling):** 主题和观察者之间是抽象依赖（依赖于 `Observer` 和 `Subject` 接口），而不是具体依赖。主题不知道观察者的具体实现，这使得它们可以独立变化。

- **支持广播通信 (Broadcast Communication):** 主题可以向任意数量的观察者广播通知，而无需修改主题的代码。

- **可扩展性 (Extensibility):** 可以很容易地添加新的具体观察者，而无需修改主题的代码，符合**开闭原则**。

# 两种变体

1. **推模型 (Push Model):**

- 主题将**所有相关数据**（或变更数据）作为参数，推给观察者的 `Update` 方法。
- **优点:** 效率高，观察者立即获得所需数据。
- **缺点:** 主题可能会传递观察者不需要的数据，浪费资源。

2. **拉模型 (Pull Model):**

- 主题只通知观察者**有状态变化**，观察者的 `Update` 方法通常不带参数或只带一个主题引用。
- 观察者收到通知后，主动调用主题的公共方法来**拉取**它感兴趣的数据。
- **优点:** 观察者只拉取需要的数据。
- **缺点:** 需要额外的网络/系统调用来获取数据，效率可能略低。