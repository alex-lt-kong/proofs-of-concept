#include "singleton-impl.h"

#include <benchmark/benchmark.h>

#include <charconv>
#include <random>
#include <iostream>

std::vector<int32_t> generate_random_values(size_t size) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int32_t> dist(INT32_MIN, INT32_MAX);

    std::vector<int32_t> values(size);
    for (size_t i = 0; i < size; ++i) {
        values[i] = dist(gen);
    }
    return values;
}

std::vector random_values = generate_random_values(1000);

static void BM_MySingletonMeyers(benchmark::State &state) {
    for (auto _: state) {
        for (const auto &value: random_values) {
            MySingletonMeyers &singleton0 = MySingletonMeyers::instance();
            MySingletonMeyers &singleton1 = MySingletonMeyers::instance();
            singleton0.set_value(value);
            if (value != singleton1.get_value()) {
                throw std::logic_error("");
            }
        }
    }
}

BENCHMARK(BM_MySingletonMeyers);

static void BM_MySingletonMutex(benchmark::State &state) {
    for (auto _: state) {
        for (const auto &value: random_values) {
            MySingletonMutex &singleton0 = MySingletonMutex::instance();
            MySingletonMutex &singleton1 = MySingletonMutex::instance();
            singleton0.set_value(value);
            if (value != singleton1.get_value()) {
                throw std::logic_error("");
            }
        }
    }
}

BENCHMARK(BM_MySingletonMutex);

static void BM_MySingletonCallOnce(benchmark::State &state) {
    for (auto _: state) {
        for (const auto &value: random_values) {
            MySingletonCallOnce &singleton0 = MySingletonCallOnce::instance();
            MySingletonCallOnce &singleton1 = MySingletonCallOnce::instance();
            singleton0.set_value(value);
            if (value != singleton1.get_value()) {
                throw std::logic_error("");
            }
        }
    }
}

BENCHMARK(BM_MySingletonCallOnce);

int main(int argc, char **argv) {

    random_values.push_back(0);
    random_values.push_back(1);
    random_values.push_back(-1);
    random_values.push_back(INT32_MAX);
    random_values.push_back(INT32_MIN);

    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();

    return 0;
}
// BENCHMARK_MAIN();