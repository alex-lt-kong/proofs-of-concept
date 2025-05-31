#include "library.h"

#include <benchmark/benchmark.h>

#include <charconv>
#include <stdexcept>
#include <random>

std::vector<int64_t> generate_random_values(size_t size) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int64_t> dist(INT64_MIN, INT64_MAX);

    std::vector<int64_t> values(size);
    for (size_t i = 0; i < size; ++i) {
        values[i] = dist(gen);
    }
    return values;
}

std::vector random_values = generate_random_values(1000);

static void BM_std_to_chars(benchmark::State &state) {
    size_t iter = 0;
    for (auto _: state) {
        char buffer[BUFFER_SIZE] = {0};
        auto result = std::to_chars(buffer, buffer + sizeof(buffer),
                                    random_values[iter % random_values.size()]);

        if (result.ec == std::errc()) {
        } else {
            throw std::logic_error("");
        }
    }
}

BENCHMARK(BM_std_to_chars);

static void BM_my_int64_to_char_v1(benchmark::State &state) {
    for (auto _: state) {
        size_t iter = 0;
        char buffer[BUFFER_SIZE] = {0};
        int64_to_char_v1(random_values[iter % random_values.size()], buffer);
    }
}

BENCHMARK(BM_my_int64_to_char_v1);

static void BM_my_int64_to_char_v2(benchmark::State &state) {
    for (auto _: state) {
        size_t iter = 0;
        char buffer[BUFFER_SIZE] = {0};
        int64_to_char_v2(random_values[iter % random_values.size()], buffer);
    }
}

BENCHMARK(BM_my_int64_to_char_v2);

static void BM_my_int64_to_char_v3(benchmark::State &state) {
    for (auto _: state) {
        size_t iter = 0;
        char buffer[BUFFER_SIZE] = {0};
        int64_to_char_v3(random_values[iter % random_values.size()], buffer);
    }
}

BENCHMARK(BM_my_int64_to_char_v3);


int main(int argc, char **argv) {

    random_values.push_back(0);
    random_values.push_back(1);
    random_values.push_back(-1);
    random_values.push_back(INT64_MAX);
    random_values.push_back(INT64_MIN);

    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();

    return 0;
}
// BENCHMARK_MAIN();