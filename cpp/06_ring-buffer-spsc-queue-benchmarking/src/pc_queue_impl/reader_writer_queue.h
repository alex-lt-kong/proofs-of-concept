// https://github.com/cameron314/readerwriterqueue

#include "../pc_queue.h"

#include <readerwriterqueue/readerwriterqueue.h>

using rwq = moodycamel::ReaderWriterQueue<uint32_t, 1024 * 1024>;

template <> inline void PcQueue<rwq>::consume() {
  uint32_t msg;
  while (true) {
    if (msg_count < m_enqueue_iterations) [[likely]] {
      if (q.try_dequeue(msg)) [[likely]] {
        ++msg_count;
        m_cb_on_dequeue(msg, msg_count);
      }
    } else [[unlikely]] {
      break;
    }
  }
}

template <> inline void PcQueue<rwq>::enqueue(uint32_t msg) { q.enqueue(msg); }
