// https://github.com/max0x7ba/atomic_queue

#include "../pc_queue.h"

#include <atomic_queue/atomic_queue.h>

// The below follows its own benchmark impl:
// https://github.com/max0x7ba/atomic_queue/blob/master/src/benchmarks.cc
using Element = uint32_t; // Queue element type.
Element constexpr NIL =
    static_cast<Element>(-1); // Atomic elements require a special value that
                              // cannot be pushed/popped.
using aqb = atomic_queue::AtomicQueue<Element, QUEUE_SIZE / 32, NIL, true, true,
                                      false, true>;

template <> inline void PcQueue<aqb>::consume() {
  while (true) {
    if (msg_count < m_enqueue_iterations) [[likely]] {
      // What if q is empty()? The doc doesn't make it very clear...
      const uint32_t msg = q.pop();
      ++msg_count;
      m_cb_on_dequeue(msg, msg_count);
    } else [[unlikely]] {
      break;
    }
  }
}

template <> inline void PcQueue<aqb>::enqueue(uint32_t msg) { q.push(msg); }
