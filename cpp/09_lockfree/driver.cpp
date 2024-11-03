#include "linked-list-impl.h"
#include "naive-impl.h"

#include <fmt/format.h>

#include <iostream>
#include <memory>

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
void producer_func(Q &q) {
  int count = 10;
  for (int i = 0; i < count; ++i) {
    q.push(i);
    // std::cout << "push()ed\n";
  }
}

int main() {
  int count = 10;

  LinkedList::NaiveQueue<int> nq;
  producer_func<LinkedList::NaiveQueue<int>, int>(nq);
  for (int i = 0; i < count; ++i) {
    int res;
    nq.pop(res);
    fmt::print("{}, ", res);
  }
  fmt::print("\n");

  LinkedList::LockFreeQueue<int> lfq;
  producer_func<LinkedList::LockFreeQueue<int>, int>(lfq);
  for (int i = 0; i < count; ++i) {
    int res;
    lfq.pop(res);
    fmt::print("{}, ", res);
  }
  fmt::print("\n");
}