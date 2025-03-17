#include "ringbuffer-1p1c-impl.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <print>
#include <utility>

TEST(RingBuffer1P1C, SingleThreadBasicProduceThenConsume) {
    constexpr std::size_t sz = 63356;
    PoC::LockFree::RingBuffer1P1C<int> rb(sz);
    for (std::size_t i = 0; i < sz; i++) {
        EXPECT_TRUE(rb.enqueue(i));
    }
    for (std::size_t i = 0; i < sz; i++) {
        auto ele = rb.dequeue();
        EXPECT_TRUE(ele.has_value());
        EXPECT_EQ(ele.value(), i);
    }
}

TEST(RingBuffer1P1C, SingleThreadProduceAndConsume) {
    constexpr std::size_t sz = 1;
    PoC::LockFree::RingBuffer1P1C<int> rb(sz);
    for (std::size_t i = 0; i < INT32_MAX; i++) {
        EXPECT_TRUE(rb.enqueue(i));
        auto ele = rb.dequeue();
        EXPECT_TRUE(ele.has_value());
        EXPECT_EQ(ele.value(), i);
    }
}

TEST(RingBuffer1P1C, SingleThreadProduceOverflow) {
    constexpr std::size_t sz = INT8_MAX;
    PoC::LockFree::RingBuffer1P1C<int> rb(sz);
    for (std::size_t i = 0; i < sz * 2; i++) {
        if (i < sz)
            EXPECT_TRUE(rb.enqueue(i));
        else
            EXPECT_FALSE(rb.enqueue(i));
    }
}

TEST(RingBuffer1P1C, SingleThreadConsumeUnderflow) {
    constexpr std::size_t sz = INT8_MAX;
    PoC::LockFree::RingBuffer1P1C<int> rb(sz);
    for (std::size_t i = 0; i < sz * 2; i++) {
        if (i < sz)
            EXPECT_TRUE(rb.enqueue(i));
        else
            EXPECT_FALSE(rb.enqueue(i));
    }
    for (std::size_t i = 0; i < sz * 2; i++) {
        auto ele = rb.dequeue();
        if (i < sz) {
            EXPECT_TRUE(ele.has_value());
            EXPECT_EQ(ele.value(), i);
        } else {
            EXPECT_FALSE(ele.has_value());
        }
    }
}
