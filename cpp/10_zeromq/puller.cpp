#include <csignal>
#include <iostream>
#include <print>
#include <thread>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "utils.h"

std::mutex mtx;

void PrintStats(PercentileTracker<int64_t> &pt) {
    double msg_per_sec = 0.0;
    const std::vector threeSigmas = {68.0, 95.0, 99.7};
    std::vector<int64_t> stats;
    auto t0 = chrono::duration_cast<chrono::microseconds>(
        chrono::high_resolution_clock::now().time_since_epoch()).count();
    while (!ev_flag) {
        std::this_thread::sleep_for(std::chrono::seconds(3)); {
            pt.refreshStats();
            stats = pt.getPercentiles(threeSigmas);
        }
        const auto now = chrono::duration_cast<chrono::microseconds>(
        chrono::high_resolution_clock::now().time_since_epoch()).count();
        msg_per_sec = 1'000'000 * msgCount / (now - t0);
        std::print(
            "MsgCount: {}k ({}/s); Percentiles (us) from {} samples at {} sampling rate: 68th: {}, 95th: {}, 99.7th: {}\n",
            msgCount / 1000, msg_per_sec, pt.sampleCount(),
            pt.sample_ratio, stats[0], stats[1], stats[2]);
    }
}

int main() {
    signal(SIGINT, signal_handler);
    auto pt = PercentileTracker<int64_t>(1000, 0.01, mtx);
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::pull);
    sock.connect("tcp://127.0.0.1:31415");

    std::thread thStats(PrintStats, std::ref(pt));
    while (!ev_flag) {
        zmq::message_t msg;
        if (!sock.recv(msg, zmq::recv_flags::none))
            break;
        ++msgCount;
        auto pusherTs = *msg.data<int64_t>();
        const int64_t pullerTs = chrono::duration_cast<chrono::microseconds>(
            chrono::high_resolution_clock::now().time_since_epoch()).count();

        pt.addSample(pullerTs - pusherTs);
    }
    thStats.join();
    return 0;

}