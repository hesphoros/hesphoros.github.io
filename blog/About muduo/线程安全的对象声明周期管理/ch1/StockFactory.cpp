#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <iostream>
#include <map>
#include <string>
#include <functional>
#include "NoCopyAble.hpp"
#include "MutexLockGuard.hpp"
#include "MutexLock.hpp"

class Stock{
public:
    explicit Stock(const std::string& stock_key)
        : m_stockKey(stock_key) {
        // 初始化股票对象
        std::cout << "Stock constructor: " << stock_key << std::endl;
    }

    std::string getKey() const {
        return m_stockKey;
    }

    ~Stock() {
        std::cout << "Stock destructor: " << m_stockKey << std::endl;
    }

private:
    std::string m_stockKey;
};

using namespace std;

//version1 : problemable code
// m_stocks 的生命周期和 StockFactory 一样长， 无法自动释放,即使外部已经不再使用
class StockFactory  : private NoCopyAble {
    using StockPtr = std::shared_ptr<Stock>;
public:
    StockPtr get(const string& stock_key)  {
        MutexLockGuard lock(m_mutex);
        StockPtr& stock_ptr = m_stocks[stock_key];
        if (!stock_ptr) {
            stock_ptr.reset(new Stock(stock_key));
        }
        return stock_ptr;
    }
private:
    mutable MutexLock m_mutex;
    std::map<std::string, StockPtr> m_stocks;
};



// version2: using weak_ptr to break the strong reference cycle
// problem: stocks_对象只增不减,stocks_.size() 是曾经存活过的Stock对象的总数,即便活的Stock对象数目将为0.
class StockFactory2  : private NoCopyAble {
    using StockPtr      = std::shared_ptr<Stock>;
    using StockWeakPtr  = std::weak_ptr<Stock>;
public:
    StockPtr get(const string& stock_key) {
        StockPtr stock_ptr;        
        MutexLockGuard lock(m_mutex);
        StockWeakPtr& weak_ptr = m_stocks[stock_key];
        stock_ptr = weak_ptr.lock(); // 尝试提升为 shared_ptr
        if (!stock_ptr) {
            stock_ptr.reset(new Stock(stock_key));
            weak_ptr = stock_ptr; // 更新 weak_ptr
        }                
        return stock_ptr;
    }
private:
    mutable MutexLock m_mutex;
    std::map<std::string, StockWeakPtr> m_stocks;
};

// version3:clean up expired weak_ptr entries
// using shared_ptr custom deleter
class StockFactory3  : private NoCopyAble {
    using StockPtr      = std::shared_ptr<Stock>;
    using StockWeakPtr  = std::weak_ptr<Stock>;
public:
    StockPtr get(const string& stock_key) {
        StockPtr stock_ptr;
        MutexLockGuard lock(m_mutex);
        StockWeakPtr& weak_ptr = m_stocks[stock_key];
        stock_ptr = weak_ptr.lock(); // 尝试提升为 shared_ptr
        if (!stock_ptr) {
            // 使用 shared_ptr 的自定义删除器
            stock_ptr.reset(new Stock(stock_key),
                [this, stock_key](Stock* p) {
                    // 自定义删除器，清理过期的 weak_ptr 条目
                    MutexLockGuard lock(m_mutex);
                    m_stocks.erase(stock_key);
                    delete p; // 别忘了释放对象本身
                });
            weak_ptr = stock_ptr; // 更新 weak_ptr
        }
        return stock_ptr;
    }
private:
    mutable MutexLock m_mutex;
    std::map<std::string, StockWeakPtr> m_stocks;
};

// version4 : using enable_shared_from_this
// 存在问题: StockFactory4 必须通过 shared_ptr 来管理其生命周期，
// 而且其生命周期被意外延长了
class StockFactory4 : public std::enable_shared_from_this<StockFactory4>
{
    using StockPtr      = std::shared_ptr<Stock>;
    using StockWeakPtr  = std::weak_ptr<Stock>;
public:
    StockFactory4(){ };

    StockPtr get(const string& stock_key) {
        StockPtr stock_ptr;
        MutexLockGuard lock(m_mutex);
        StockWeakPtr& weak_ptr = m_stocks[stock_key];
        stock_ptr = weak_ptr.lock(); // 尝试提升为 shared_ptr
       
        if (!stock_ptr) {
            auto shared_this = shared_from_this();
            stock_ptr.reset(new Stock(stock_key),
                std::bind(&StockFactory4::DeleteCallback,
                          shared_this,
                          stock_key,
                          std::placeholders::_1));
            weak_ptr = stock_ptr; // 更新 weak_ptr
        }
        return stock_ptr;
    }
private:
    static void DeleteCallback(const std::shared_ptr<StockFactory4>& Factory,
                                   const string& stock_key,
                                   Stock* p) {
        
        if (Factory) {
            MutexLockGuard lock(Factory->m_mutex);
            Factory->m_stocks.erase(stock_key);
        }
        delete p;
    }
    
    mutable MutexLock m_mutex;
    std::map<std::string, StockWeakPtr> m_stocks;
};

std::shared_ptr<StockFactory4> factory4_ptr = std::make_shared<StockFactory4>();


// version5 : using enable_shared_from_this without custom deleter weak_ptr cleanup
class StockFactory5 : public std::enable_shared_from_this<StockFactory5>
{
    using StockPtr          = std::shared_ptr<Stock>;
    using StockWeakPtr      = std::weak_ptr<Stock>;
    using WkStockFactoryPtr = std::weak_ptr<StockFactory5>;
    using StockFactoryPtr   = std::shared_ptr<StockFactory5>;
public:
    StockFactory5(){ 
        std::cout << "StockFactory5 constructor" << std::endl;
    }

    ~StockFactory5() {
        std::cout << "StockFactory5 destructor" << std::endl;
    }

    StockPtr get(const string& stock_key) {
        StockPtr stock_ptr;
        {
            MutexLockGuard lock(m_mutex);
            StockWeakPtr& weak_ptr = m_stocks[stock_key];
            stock_ptr = weak_ptr.lock(); // 尝试提升为 shared_ptr
            if (!stock_ptr) {
                WkStockFactoryPtr weak_factory(shared_from_this());
                stock_ptr.reset(new Stock(stock_key),
                    std::bind(&StockFactory5::weakDeleteCallback,
                              weak_factory,
                              stock_key,
                              std::placeholders::_1));
                weak_ptr = stock_ptr; // 更新 weak_ptr
            }
        }
        return stock_ptr;
    }
private:

    void removeStock(Stock* p) {
        if (p) {
            MutexLockGuard lock(m_mutex);
            m_stocks.erase(p->getKey());
        }
    }

    static void weakDeleteCallback(const WkStockFactoryPtr& weak_factory,
                                   const string& stock_key,
                                   Stock* p) {
        StockFactoryPtr factory(weak_factory.lock());
        if (factory) {
            MutexLockGuard lock(factory->m_mutex);
            factory->m_stocks.erase(stock_key);
        }
        delete p;
    }

    mutable MutexLock m_mutex;
    std::map<std::string, StockWeakPtr> m_stocks;
};

void testLongLifeFactory()
{
    shared_ptr<StockFactory5> factory(new StockFactory5());
    {
        shared_ptr<Stock> stock1 = factory->get("NYSE:IBM");
        shared_ptr<Stock> stock2 = factory->get("NYSE:IBM");
        cout << "Stock1 key: " << stock1->getKey() << endl;
        cout << "Stock2 key: " << stock2->getKey() << endl;
        assert(stock1 == stock2);
        // stock destroyed here
    }
    // facory destroyed here
}

void testShortLifeFactory()
{
    shared_ptr<Stock> stock;
    {
        shared_ptr<StockFactory5> factoty(new StockFactory5());
        stock = factoty->get("NYSE:IBM");
        shared_ptr<Stock> stock2 = factoty->get("NYSE:IBM");
        cout << "Stock1 key: " << stock->getKey() << endl;
        assert(stock == stock2);
        // factory destroyed here
    }
    // stock destroyed here
}



int main() {
    testLongLifeFactory();
    testShortLifeFactory();
    return 0;
}

//========================
// g++ -std=c++11 -o StockFactory.exe StockFactory.cpp
//=====================================================

// output
// StockFactory5 constructor
// Stock constructor: NYSE:IBM
// Stock1 key: NYSE:IBM
// Stock2 key: NYSE:IBM
// Stock destructor: NYSE:IBM
// StockFactory5 destructor
// StockFactory5 constructor
// Stock constructor: NYSE:IBM
// Stock1 key: NYSE:IBM
// StockFactory5 destructor
// Stock destructor: NYSE:IBM