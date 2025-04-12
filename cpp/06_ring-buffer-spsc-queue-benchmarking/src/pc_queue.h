#ifndef PC_QUEUE_H
#define PC_QUEUE_H

#include <functional>
#include <thread>

#define QUEUE_SIZE (1024 * 1024 * 16)

template <class T_QUEUE> class PcQueue {
private:
  T_QUEUE q;
  std::thread consumer;
  size_t m_enqueue_iterations;
  size_t msg_count = 0;
  void consume();
  std::mutex mut;
  // uint32_t *element_counter;
  std::function<void(const uint32_t &ele, std::size_t msg_count)>
      m_cb_on_dequeue;

public:
  PcQueue(const size_t iterations,
          const decltype(m_cb_on_dequeue) &cb_on_dequeue) {
    // this->element_counter = element_counter;
    m_cb_on_dequeue = cb_on_dequeue;
    m_enqueue_iterations = iterations;
  }

  void start() { consumer = std::thread(&PcQueue::consume, this); }

  void wait() {
    if (consumer.joinable()) {
      consumer.join();
    }
  }

  ~PcQueue() { // wait();
  }

  void enqueue(uint32_t msg);
  size_t handled_msg_count() { return this->msg_count; }
};

#endif // RS_PC_QUEUE_H
