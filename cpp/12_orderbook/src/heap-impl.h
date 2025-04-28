#ifndef HEAP_IMPL_H
#define HEAP_IMPL_H

#include <stdexcept>
#include <vector>

namespace PoC::OrderBook {
    /* For any ith node arr[i]:
     *    - arr[(i -1) / 2] returns its parent node.
     *    - arr[(2 * i) + 1] returns its left child node.
     *    - arr[(2 * i) + 2] returns its right child node.
     */
    template<std::totally_ordered T>
    class MinHeap {
        std::vector<T> m_data;

    public:
        void insert(const T &val) {
            m_data.push_back(val);
            auto idx = m_data.size() - 1;
            while (idx > 0) {
                size_t parent = (idx - 1) / 2;
                if (m_data[idx] < m_data[parent]) {
                    std::swap(m_data[idx], m_data[parent]);
                    idx = parent;
                } else {
                    break;
                }
            }
        }

        T peek() {
            if (m_data.empty()) {
                throw std::out_of_range("Heap is empty");
            }
            return m_data[0];
        }

        void pop() {
            if (m_data.empty()) {
                throw std::out_of_range("Heap is empty");
            }
            m_data[0] = m_data.back();
            m_data.pop_back();
            size_t idx = 0;
            while (true) {
                auto left = (2 * idx) + 1;
                auto right = (2 * idx) + 2;
                if (right < m_data.size()) {
                    auto min_idx = idx;
                    if (m_data[min_idx] > m_data[left]) {
                        min_idx = left;
                    }
                    if (m_data[min_idx] > m_data[right]) {
                        min_idx = right;
                    }
                    if (min_idx != idx) {
                        std::swap(m_data[min_idx], m_data[idx]);
                        idx = min_idx;
                        continue;
                    }
                }
                if (left < m_data.size()) {
                    if (m_data[idx] > m_data[left]) {
                        std::swap(m_data[idx], m_data[left]);
                    }
                    break;
                }
                break;
            }
        }

        [[nodiscard]] size_t size() const { return m_data.size(); }
    };
} // namespace PoC::OrderBook
#endif // HEAP_IMPL_H
