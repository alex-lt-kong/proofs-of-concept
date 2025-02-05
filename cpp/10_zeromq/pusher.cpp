#include <chrono>
#include <csignal>
#include <print>
#include <thread>

#include <zmq_addon.hpp>

#include "utils.h"

void PrintStats() {
    while (!ev_flag) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::print("MsgCount: {}k\n", msgCount / 1000);
    }
}

int main() {
    signal(SIGINT, signal_handler);
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::push);
    // I want to set high water mark to 1 to avoid buffering
    sock.set(zmq::sockopt::sndhwm, 1000);
    sock.bind("tcp://127.0.0.1:31415");
    int64_t t0 = chrono::duration_cast<chrono::microseconds>(
               chrono::high_resolution_clock::now().time_since_epoch()).count();
    std::thread thStats(PrintStats);
    while (!ev_flag) {
        const auto delayUs = 1;
        const int64_t pusherTs = chrono::duration_cast<chrono::microseconds>(
            chrono::high_resolution_clock::now().time_since_epoch()).count();
        zmq::message_t msg(&pusherTs, sizeof(pusherTs));
        sock.send(msg, zmq::send_flags::none);
        ++msgCount;


        if (msgCount % 1000 == 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

    }
    std::print("Pusher exited gracefully\n");
    return 0;
}
