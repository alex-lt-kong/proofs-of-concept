#include <atomic>
#include <memory>

namespace PoC::LockFree::LinkedList {
template <typename T> class LockFreeQueue {
private:
  struct Node {
    std::shared_ptr<T> data;
    std::atomic<std::shared_ptr<Node>> next;
    Node() = default;
    explicit Node(T value) : data(std::make_shared<T>(std::move(value))) {}
  };

  std::atomic<std::shared_ptr<Node>> head;
  std::atomic<std::shared_ptr<Node>> tail;

public:
  LockFreeQueue() {
    auto dummy = std::make_shared<Node>();
    head.store(dummy);
    tail.store(dummy);
  }

  void push(T value) {
    auto newNode = std::make_shared<Node>(std::move(value));

    while (true) {
      auto last = tail.load();
      auto next = last->next.load();

      if (last == tail.load()) {
        if (next == nullptr) {
          std::shared_ptr<Node> nullptr_node;
          if (last->next.compare_exchange_weak(nullptr_node, newNode)) {
            tail.compare_exchange_weak(last, newNode);
            return;
          }
        } else {
          tail.compare_exchange_weak(last, next);
        }
      }
    }
  }

  bool pop(T &result) {
    while (true) {
      auto first = head.load();
      auto last = tail.load();
      auto next = first->next.load();

      if (first == head.load()) {
        if (first == last) {
          if (!next) {
            return false;
          }
          tail.compare_exchange_weak(last, next);
        } else {
          if (next) {
            result = *(next->data);
            if (head.compare_exchange_weak(first, next)) {
              return true;
            }
          }
        }
      }
    }
  }
};
} // namespace PoC::LockFree::LinkedList