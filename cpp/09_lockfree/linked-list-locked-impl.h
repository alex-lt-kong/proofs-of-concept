#include <memory>
#include <mutex>
#include <queue>

namespace PoC::LockFree::LinkedList {

template <typename T> class LockedQueue {
private:
  std::queue<T> q;
  std::mutex mtx;

public:
  LockedQueue(int dummy_int) {}

  void push(T const value) {
    std::lock_guard<std::mutex> lk(mtx);
    q.push(value);
  }

  bool pop(T &result) {
    std::lock_guard<std::mutex> lk(mtx);
    if (q.empty())
      return false;
    result = q.front();
    q.pop();
    return true;
  }
};

} // namespace PoC::LockFree::LinkedList