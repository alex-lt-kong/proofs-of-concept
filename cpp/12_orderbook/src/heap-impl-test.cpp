#include "heap-impl.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <queue>

#include <print>
#include <random>
#include <ranges>

using namespace PoC::OrderBook;

TEST(Heap, AFewInsertAndDeleteShouldWork) {

    MinHeap<int> h;
    EXPECT_THROW(h.pop(), std::out_of_range);
    EXPECT_THROW(h.peek(), std::out_of_range);
    h.insert(3);
    EXPECT_EQ(h.peek(), 3);
    h.insert(2);
    EXPECT_EQ(h.peek(), 2);
    h.insert(1);
    EXPECT_EQ(h.peek(), 1);
    h.pop();
    EXPECT_EQ(h.peek(), 2);
    h.pop();
    EXPECT_EQ(h.peek(), 3);
    h.pop();
    EXPECT_THROW(h.pop(), std::out_of_range);
}

TEST(Heap, MoreInsertAndDeleteShouldStillWork) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dis(std::numeric_limits<int>::min(),
                                      std::numeric_limits<int>::max());
    std::vector<int> numbers(1'000'000);
    std::ranges::generate(numbers, [&] { return dis(gen); });
    std::priority_queue<int, std::vector<int>, std::greater<>> pq;
    MinHeap<int> h;

    for (auto v: numbers) {
        pq.push(v);
        h.insert(v);
        EXPECT_EQ(h.size(), pq.size());
        EXPECT_EQ(h.peek(), pq.top());
    }
    EXPECT_EQ(h.size(), pq.size());
    while (!pq.empty()) {
        EXPECT_EQ(h.size(), pq.size());
        EXPECT_EQ(h.peek(), pq.top());
        pq.pop();
        h.pop();
    }
}
