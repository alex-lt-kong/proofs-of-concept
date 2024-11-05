#include <readerwriterqueue/readerwritercircularbuffer.h>

namespace PoC::LockFree::ThirdParty {

template <typename T> class MoodyCamelReaderWriterQueue {
private:
  moodycamel::BlockingReaderWriterCircularBuffer<T> q;

public:
  MoodyCamelReaderWriterQueue(int qsize) : q(qsize) {}

  void push(T const value) { q.try_enqueue(value); }

  bool pop(T &result) { return q.try_dequeue(result); }
};

} // namespace PoC::LockFree::ThirdParty