#ifndef PERCENTILE_TRACKER_H
#define PERCENTILE_TRACKER_H

#include <algorithm>
#include <ranges>
#include <deque>
#include <random>
#include <stdexcept>


namespace chrono = std::chrono;
namespace views = std::views;
namespace ranges = std::ranges;


inline bool volatile ev_flag = false;

inline void signal_handler(int signal) {
    ev_flag = true;
}

template<class T>
class PercentileTracker {
    static_assert(std::is_arithmetic_v<T>, "T must be a numeric type");

public:
    explicit PercentileTracker(const size_t sample_size, const double sample_ratio, std::mutex& mtx) : sample_size(sample_size),
        refreshStatsCalled(false), sample_ratio(sample_ratio), mtx(mtx) {
        rnd = std::uniform_real_distribution<>(0, 1);
    }

    bool addSample(T num) {
        if (rnd(re) > sample_ratio)
            return false; {
            std::lock_guard lck(mtx);
            samples.push_back(num);
            if (samples.size() > sample_size) {
                samples.pop_front();
            }
        }


        return true;
    }

    void refreshStats() {
        std::sort(samples.begin(), samples.end());
        refreshStatsCalled = true;
    }

    std::vector<T> getPercentiles(const std::vector<double> &percents) {
        if (samples.empty())
            throw std::invalid_argument("samples.empty()");
        refreshStats();

        auto percentiles = percents | views::transform([&](auto p) {
            int index = p
            / 100.0 * samples.size() - 1;
            if (index < 0)
                index = 0;
            return samples[index];
        }) | ranges::to<std::vector>();
        return percentiles;
    }

    auto sampleCount() { return samples.size(); }

private:
    // Need to use std::deque to have std::sort() operational
    std::deque<T> samples;
    size_t sample_size;
    bool refreshStatsCalled;
    double sample_ratio;
    std::uniform_real_distribution<double> rnd;
    std::default_random_engine re;
    std::mutex &mtx;
};

#endif //PERCENTILE_TRACKER_H
