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
    }

    std::string getKey() const {
        return m_stockKey;
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