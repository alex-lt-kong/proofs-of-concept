#include <iostream>
#include <print>
#include <thread>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "utils.h"

std::mutex mtx;

void PrintStats(PercentileTracker<int64_t> &pt) {
    const std::vector threeSigmas = {68.0, 95.0, 99.7};
    std::vector<int64_t> stats;
    while (!ev_flag) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        {
            pt.refreshStats();
            stats = pt.getPercentiles(threeSigmas);

        }
        // I want to repeat the above, but print in one line:
        std::print("Percentiles from {} samples: 68th: {}us, 95th: {:.1f}us, 99.7th: {:.1f}ms\n", pt.sampleCount(),
                   stats[0], stats[1], stats[2] / 1000., 0);
    }
}

int main() {
    signal(SIGINT, signal_handler);
    auto pt = PercentileTracker<int64_t>(1000, 0.01, mtx);
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::pull);
    sock.connect("tcp://127.0.0.1:31415");


    uint32_t msgCount = 0;

    std::thread thStats(PrintStats, std::ref(pt));
    while (!ev_flag) {
        ++msgCount;
        zmq::message_t msg;
        if (!sock.recv(msg, zmq::recv_flags::none))
            break;
        ++msgCount;
        auto pusherTs = *msg.data<int64_t>();
        const int64_t pullerTs = chrono::duration_cast<chrono::microseconds>(
            chrono::high_resolution_clock::now().time_since_epoch()).count();
        if (msgCount % 1'000'000 == 0)
            std::print("Received message {} with timestamp {}, pullerTs: {}, latency: {}\n", msgCount, pusherTs, pullerTs, pullerTs - pusherTs);

        pt.addSample(pullerTs - pusherTs);

    }
    thStats.join();
    return 0;
}
