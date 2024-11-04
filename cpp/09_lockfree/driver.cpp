#include "linked-list-impl.h"
#include "linked-list-locked-impl.h"

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
void benchmarker(Q &queue) {
  constexpr int thread_count = 50;
  constexpr int count = 10000;
  std::array<int, count> hist;
  hist.fill(0);
  std::array<std::thread, count> threads;
  auto t0 = std::chrono::steady_clock::now();
  for (int i = 0; i < thread_count; ++i) {
    threads[i] = std::thread(&producer_func<Q, T>, std::ref(queue), count);
  }
  for (int i = 0; i < thread_count; ++i) {
    threads[i].join();
  }
  auto t1 = std::chrono::steady_clock::now();
  fmt::print(
      "takes: {} us\n",
      std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count());

  for (int i = 0; i < thread_count * count; ++i) {
    int res;
    queue.pop(res);
    ++hist[res];
  }
  int idx = 0;
  for (; idx < count; ++idx) {
    if (hist[idx] != thread_count) {
      fmt::print("Unexpected result hist[{}] == {}, expect {}", idx, hist[idx],
                 thread_count);
      break;
    }
  }
  if (idx == count) {
    fmt::print("Nothing unexpected!\n");
  }
  fmt::print("\n");
}

int main() {
  LinkedList::NaiveQueue<int> nq;
  benchmarker<decltype(nq), int>(nq);

  LinkedList::LockFreeQueue<int> lfq;
  benchmarker<decltype(lfq), int>(lfq);

  boost::lockfree::queue<int> blfq(100000);
  benchmarker<decltype(blfq), int>(blfq);
}