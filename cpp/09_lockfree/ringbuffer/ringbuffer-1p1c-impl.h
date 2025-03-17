#ifndef INC_1P1C_RINGBUFFER_IMPL_H
#define INC_1P1C_RINGBUFFER_IMPL_H

#include <atomic>
#include <iostream>
#include <vector>
#include <optional>

namespace PoC::LockFree {
    template<typename T>
    class RingBuffer1P1C {
    public:
        explicit RingBuffer1P1C(size_t capacity)
        // we want to distinguish between buffer empty and buffer full, so we need the allocate capacity+1
            : m_capacity(capacity + 1), m_buffer(capacity + 1), m_head(0), m_tail(0) {
        }

        bool enqueue(const T &item) {
            size_t head = m_head.load(std::memory_order_relaxed);
            size_t nextHead = (head + 1) % m_capacity;

            if (nextHead == m_tail.load(std::memory_order_acquire)) {
                return false; // Buffer is full
            }

            m_buffer[head] = item;
            m_head.store(nextHead, std::memory_order_release);
            return true;
        }


        std::optional<T> dequeue() {
            size_t tail = m_tail.load(std::memory_order_relaxed);

            // Check if the buffer is empty
            if (tail == m_head.load(std::memory_order_acquire)) {
                return std::nullopt; // Buffer is empty
            }

            T item = m_buffer[tail];
            m_tail.store((tail + 1) % m_capacity, std::memory_order_release);
            return item;
        }

    private:
        const size_t m_capacity;
        std::vector<T> m_buffer;
        std::atomic<size_t> m_head; // Points to the next position to write
        std::atomic<size_t> m_tail; // Points to the next position to read
    };
}

#endif //INC_1P1C_RINGBUFFER_IMPL_H
