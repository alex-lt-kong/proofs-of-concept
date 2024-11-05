#include <concurrentqueue/moodycamel/concurrentqueue.h>

namespace PoC::LockFree::ThirdParty {

template <typename T> class MoodyCamelConcurrentQueue {
private:
  moodycamel::ConcurrentQueue<T> q;

public:
  MoodyCamelConcurrentQueue(int dummy_int) {}

  void push(T const value) { q.enqueue(value); }

  bool pop(T &result) { return q.try_dequeue(result); }
};

} // namespace PoC::LockFree::ThirdParty