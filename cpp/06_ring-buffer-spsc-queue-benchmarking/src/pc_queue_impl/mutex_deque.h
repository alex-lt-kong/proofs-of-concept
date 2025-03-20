#include "../pc_queue.h"

#include <deque>
#include <mutex>

using dq = std::deque<uint32_t>;

template <> inline void PcQueue<dq>::consume() {
  while (true) {
    std::lock_guard<std::mutex> lk(mut);
    if (msg_count < m_enqueue_iterations) [[likely]] {
      if (!q.empty()) [[likely]] {
        const uint32_t msg = q.front();
        q.pop_front();
        ++msg_count;
        m_cb_on_dequeue(msg, msg_count);
      }
    } else [[unlikely]] {
      break;
    }
  }
}

template <> inline void PcQueue<dq>::enqueue(uint32_t msg) {
  std::lock_guard<std::mutex> lk(mut);
  q.push_back(msg);
}
