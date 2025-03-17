#include "ringbuffer-1p1c-impl.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <print>
#include <utility>

TEST(RingBuffer1P1C, SingleThreadBasicProduceThenConsume) {
  constexpr std::size_t sz = 63356;
  PoC::LockFree::RingBuffer1P1C<int> rb(sz);

  auto ele = rb.dequeue();
  EXPECT_FALSE(ele.has_value());

  for (std::size_t i = 0; i < sz; i++) {
    EXPECT_TRUE(rb.enqueue(i));
  }
  for (std::size_t i = 0; i < sz; i++) {
    auto ele = rb.dequeue();
    EXPECT_TRUE(ele.has_value());
    EXPECT_EQ(ele.value(), i);
  }
  ele = rb.dequeue();
  EXPECT_FALSE(ele.has_value());
}

TEST(RingBuffer1P1C, SingleThreadProduceAndConsume) {
  constexpr std::size_t sz = 1;
  PoC::LockFree::RingBuffer1P1C<int> rb(sz);
  for (std::size_t i = 0; i < INT16_MAX; i++) {
    EXPECT_TRUE(rb.enqueue(i));
    auto ele = rb.dequeue();
    EXPECT_TRUE(ele.has_value());
    EXPECT_EQ(ele.value(), i);
  }
}

TEST(RingBuffer1P1C, SingleThreadProduceOverflow) {
  constexpr std::size_t sz = INT8_MAX;
  PoC::LockFree::RingBuffer1P1C<int> rb(sz);
  for (std::size_t i = 0; i < INT16_MAX; i++) {
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

TEST(RingBuffer1P1C, 1P1CSimple) {
  constexpr std::size_t sz = INT32_MAX / 4;
  constexpr std::size_t iter_size = INT32_MAX;
  PoC::LockFree::RingBuffer1P1C<int> rb(sz);
  auto producer = [&] {
    for (std::size_t i = 0; i < iter_size; ++i) {
      EXPECT_TRUE(rb.enqueue(i));
    }
  };
  auto consumer = [&] {
    std::size_t dequeue_count = 0;
    while (dequeue_count < iter_size) {
      auto ele = rb.dequeue();
      if (ele.has_value()) {
        EXPECT_EQ(ele.value(), dequeue_count);
        ++dequeue_count;
      }
    }
  };

  std::thread thread1(producer);
  std::thread thread2(consumer);

  thread1.join();
  thread2.join();

  const auto ele = rb.dequeue();
  EXPECT_FALSE(ele.has_value());
}