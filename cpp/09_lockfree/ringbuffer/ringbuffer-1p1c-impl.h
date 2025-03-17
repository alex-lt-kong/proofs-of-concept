#ifndef INC_1P1C_RINGBUFFER_IMPL_H
#define INC_1P1C_RINGBUFFER_IMPL_H

#include <atomic>
#include <optional>
#include <vector>

/* Notes:
 * - load() always comes with std::memory_order_acquire, aka, read-acquire
 * - store() always comes with std::memory_order_release, aka, write-release
 * - read-acquire and write-release are logical memory barriers
 * - A read-acquire guarantees that all following code starts after the
 * acquiring read
 * - A write-release guarantees that all preceding code completes before the
 * releasing write
 */
namespace PoC::LockFree {
template <typename T> class RingBuffer1P1C {
public:
  explicit RingBuffer1P1C(const size_t capacity)
      // we want to distinguish between buffer empty and buffer full, so we need
      // the allocate capacity+1
      : m_capacity(capacity + 1), m_buffer(capacity + 1), m_write_ptr(0),
        m_read_ptr(0) {}

  bool enqueue(const T &item) {
    size_t tail = m_write_ptr.load(std::memory_order_relaxed);
    const size_t next_tail = (tail + 1) % m_capacity;

    if (next_tail == m_read_ptr.load(std::memory_order_acquire)) {
      return false; // Buffer is full
    }

    m_buffer[tail] = item;
    m_write_ptr.store(next_tail, std::memory_order_release);
    return true;
  }

  std::optional<T> dequeue() {
    size_t head = m_read_ptr.load(std::memory_order_relaxed);
    if (head == m_write_ptr.load(std::memory_order_acquire)) {
      return std::nullopt; // Buffer is empty
    }

    T item = m_buffer[head];
    m_read_ptr.store((head + 1) % m_capacity, std::memory_order_release);
    return item;
  }

  [[nodiscard]] std::size_t size_approx() const {
    const size_t tail = m_write_ptr.load(std::memory_order_relaxed);
    const size_t head = m_read_ptr.load(std::memory_order_relaxed);
    if (tail >= head)
      return tail - head;
    return (m_capacity + tail - head) % m_capacity;
  }

private:
  const size_t m_capacity;
  std::vector<T> m_buffer;
  std::atomic<size_t> m_write_ptr; // Points to the next position to write
  std::atomic<size_t> m_read_ptr;  // Points to the next position to read
};
} // namespace PoC::LockFree

#endif // INC_1P1C_RINGBUFFER_IMPL_H
