#include "library.h"

#include <gtest/gtest.h>

#include <random>
#include <charconv>

TEST(LibraryTest, Simple) {
    char buffer0[BUFFER_SIZE];
    char buffer1[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];
    char buffer3[BUFFER_SIZE];
    std::vector<int64_t> values = {3, 17, 23334, -3};
    for (const auto &value: values) {
        memset(buffer0, 0, BUFFER_SIZE);
        memset(buffer1, 0, BUFFER_SIZE);
        memset(buffer2, 0, BUFFER_SIZE);
        memset(buffer3, 0, BUFFER_SIZE);
        auto result = std::to_chars(buffer0, buffer0 + BUFFER_SIZE,
                                    value);
        int64_to_char_v1(value, buffer1);
        int64_to_char_v2(value, buffer2);
        int64_to_char_v2(value, buffer3);
        if (result.ec != std::errc()) {
            throw std::logic_error("");
        }
        EXPECT_EQ(strcmp(buffer0, buffer1), 0)
                            << "buffer0: " << buffer0 << ", buffer1: "
                            << buffer1;
        EXPECT_EQ(strcmp(buffer0, buffer2), 0)
                            << "buffer0: " << buffer0 << ", buffer2: "
                            << buffer2;
        EXPECT_EQ(strcmp(buffer0, buffer3), 0)
                            << "buffer0: " << buffer0 << ", buffer3: "
                            << buffer3;
    }
}

TEST(LibraryTest, Random) {
    char buffer0[BUFFER_SIZE];
    char buffer1[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];
    char buffer3[BUFFER_SIZE];
    constexpr size_t test_count = 1'000'000;
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::normal_distribution<double> dist(0.0, 1'000'000.0);


    for (int i = 0; i < test_count; ++i) {

        auto random_value = static_cast<int64_t>(std::round(dist(gen)));
        // std::cout << "testing: " << random_value << "\n";
        memset(buffer0, 0, BUFFER_SIZE);
        memset(buffer1, 0, BUFFER_SIZE);
        memset(buffer2, 0, BUFFER_SIZE);
        memset(buffer3, 0, BUFFER_SIZE);
        auto result = std::to_chars(buffer0, buffer0 + BUFFER_SIZE,
                                    random_value);
        if (result.ec != std::errc()) {
            throw std::logic_error("");
        }
        int64_to_char_v1(random_value, buffer1);
        int64_to_char_v2(random_value, buffer2);
        int64_to_char_v3(random_value, buffer3);
        EXPECT_EQ(strcmp(buffer0, buffer1), 0)
                            << "buffer0: " << buffer0 << ", buffer1: "
                            << buffer1;
        EXPECT_EQ(strcmp(buffer0, buffer2), 0)
                            << "buffer0: " << buffer0 << ", buffer2: "
                            << buffer2;
        EXPECT_EQ(strcmp(buffer0, buffer3), 0)
                            << "buffer0: " << buffer0 << ", buffer3: "
                            << buffer3;
    }
}

TEST(LibraryTest, EdgeCases) {
    char buffer0[BUFFER_SIZE];
    char buffer1[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];
    char buffer3[BUFFER_SIZE];

    std::vector<int64_t> values = {0, 1, -1, INT64_MAX, INT64_MIN};
    for (const auto &value: values) {
        memset(buffer0, 0, BUFFER_SIZE);
        memset(buffer1, 0, BUFFER_SIZE);
        memset(buffer2, 0, BUFFER_SIZE);
        memset(buffer3, 0, BUFFER_SIZE);
        auto result = std::to_chars(buffer0, buffer0 + BUFFER_SIZE,
                                    value);
        if (result.ec != std::errc()) {
            throw std::logic_error("");
        }
        int64_to_char_v1(value, buffer1);
        int64_to_char_v2(value, buffer2);
        int64_to_char_v3(value, buffer3);
        EXPECT_EQ(strcmp(buffer0, buffer1), 0) << "buffer1: " << buffer1;
        EXPECT_EQ(strcmp(buffer0, buffer2), 0) << "buffer2: " << buffer2;
        EXPECT_EQ(strcmp(buffer0, buffer3), 0) << "buffer3: " << buffer3;
    }

}