#include <memory>
#include <mutex>

namespace PoC::LockFree::LinkedList {

template <typename T> class NaiveQueue {
private:
  struct Node {
    std::unique_ptr<T> m_data;
    std::unique_ptr<Node> m_next;
    Node(T const &data) : m_data(std::make_unique<T>(data)) {}
  };

  std::mutex mtx;
  std::unique_ptr<Node> head;
  Node *tail; // Raw pointer for rear since it doesn't own the node
  int size;

public:
  NaiveQueue() : head(nullptr), tail(nullptr), size(0) {}

  void push(T const value) {
    auto new_node = std::make_unique<Node>(value);
    {
      std::lock_guard<std::mutex> lg(mtx);
      if (empty()) {
        head = std::move(new_node);
        tail = head.get();
      } else {
        tail->m_next = std::move(new_node);
        tail = tail->m_next.get();
      }
      size++;
    }
  }

  bool pop(T &result) {
    std::lock_guard<std::mutex> lg(mtx);
    if (empty()) {
      return false;
    }

    result = *(head->m_data);
    head = std::move(head->m_next);

    if (head == nullptr) {
      return false;
    }

    size--;
    return true;
  }

  T *peek() const {
    if (empty()) {
      return nullptr;
    }
    return head->m_data.get();
  }

  bool empty() const { return head == nullptr; }

  int get_size() const { return size; }
};

} // namespace PoC::LockFree::LinkedList