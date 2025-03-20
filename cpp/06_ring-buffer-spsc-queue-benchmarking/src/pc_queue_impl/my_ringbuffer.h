#include "../../../09_lockfree/ringbuffer/ringbuffer-spsc-impl.h"

#include "../pc_queue.h"

#include <print>

using myrb = PoC::LockFree::RingBufferSPSC<uint32_t>;

template <>
inline PcQueue<myrb>::PcQueue(size_t iterations,
                              const decltype(m_cb_on_dequeue) &cb_on_dequeue)
    : q(QUEUE_SIZE) {
  m_cb_on_dequeue = cb_on_dequeue;
  this->m_enqueue_iterations = iterations;
}

template <> inline void PcQueue<myrb>::consume() {
  uint32_t msg;
  while (true) {
    if (msg_count < m_enqueue_iterations) [[likely]] {
      if (q.dequeue(msg)) [[likely]] {
        ++msg_count;
        m_cb_on_dequeue(msg, msg_count);
      }
    } else [[unlikely]] {
      break;
    }
  }
}

template <> inline void PcQueue<myrb>::enqueue(uint32_t msg) {
  if (!q.enqueue(std::move(msg))) {
    std::print("enqueueFailed\n");
  }
}
