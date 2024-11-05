#include "concurrent-queue-wrapper.h"
#include "linked-list-impl.h"
#include "linked-list-locked-impl.h"
#include "reader-writer-queue-wrapper.h"

#include <boost/lockfree/queue.hpp>
#include <fmt/format.h>

#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

using namespace PoC::LockFree;

// We defined a concept "is_shape" that requires an object of type T has a
// member function called area(), which returns a floating point variable
template <typename Q, typename T>
concept is_queue = requires(Q q, const T &value, T &result) {
  { q.push(value) };
  { q.pop(result) } -> std::same_as<bool>;
};

template <typename Q, typename T>
  requires is_queue<Q, T>
void producer_func(Q &q, int push_size) {
  for (int i = 0; i < push_size; ++i) {
    q.push(i);
  }
}

template <typename Q, typename T>
  requires is_queue<Q, T>
void consumer_func(Q &q, std::vector<int> &hist, int procuder_thread_count,
                   int ele_count) {
  int poped_count = 0;
  while (poped_count < procuder_thread_count * ele_count) {
    int res;
    if (q.pop(res)) {
      ++poped_count;
      ++hist[res];
    }
    // fmt::print("{} vs {}\n", poped_count, procuder_thread_count * ele_count);
  }
}

template <typename Q, typename T>
  requires is_queue<Q, T>
void benchmarker() {
  constexpr int thread_count = 1;
  constexpr int count = 1000000;
  std::vector<int> hist(count, 0);
  std::array<std::thread, count> threads;
  Q queue(thread_count * count);
  auto t0 = std::chrono::steady_clock::now();
  for (int i = 0; i < thread_count; ++i) {
    threads[i] = std::thread(&producer_func<Q, T>, std::ref(queue), count);
  }
  consumer_func<Q, T>(queue, hist, thread_count, count);
  for (int i = 0; i < thread_count; ++i) {
    threads[i].join();
  }
  auto t1 = std::chrono::steady_clock::now();
  fmt::print(
      "takes: {} ms\n",
      std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());

  for (int idx = 0; idx < count; ++idx) {
    if (hist[idx] != thread_count) {
      fmt::print("Unexpected result hist[{}] == {}, expect {}", idx, hist[idx],
                 thread_count);
      break;
    }
  }
}

int main() {
  benchmarker<LinkedList::LockedQueue<int>, int>();
  benchmarker<LinkedList::LockFreeQueue<int>, int>();
  benchmarker<boost::lockfree::queue<int>, int>();
  benchmarker<ThirdParty::MoodyCamelConcurrentQueue<int>, int>();
  benchmarker<ThirdParty::MoodyCamelReaderWriterQueue<int>, int>();
}