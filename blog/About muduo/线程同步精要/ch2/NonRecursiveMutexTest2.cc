#include "../../common/MutexLock.hpp"
#include "../../common/MutexLockGuard.hpp"
#include "../../common/Thread.hpp"
#include <set>
#include <vector>
#include <stdio.h>
#include <memory>

class  Request;
typedef std::shared_ptr<Request> RequestPtr;

// 库存清单
class Inventory
{
 public:
  Inventory()
    : requests_(new RequestList)
  {
  }

  void add(Request* req)
  {
    MutexLockGuard lock(mutex_);
    if (!requests_.unique())
    {
      requests_.reset(new RequestList(*requests_));
      printf("Inventory::add() copy the whole list\n");
    }
    assert(requests_.unique());
    requests_->insert(req);
  }

  void remove(Request* req) // __attribute__ ((noinline))
  {
    MutexLockGuard lock(mutex_);
    if (!requests_.unique())
    {
      requests_.reset(new RequestList(*requests_));
      printf("Inventory::remove() copy the whole list\n");
    }
    assert(requests_.unique());
    requests_->erase(req);
  }

  void printAll() const;

 private:
  typedef std::set<Request*> RequestList;
  typedef std::shared_ptr<RequestList> RequestListPtr;

  RequestListPtr getData() const
  {
    MutexLockGuard lock(mutex_);
    return requests_;
  }

  mutable MutexLock mutex_;
  RequestListPtr requests_;
};