#include <chrono>
#include <print>
#include <thread>

#include <zmq_addon.hpp>

#include "utils.h"

int main() {
    signal(SIGINT, signal_handler);
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::push);
    // I want to set high water mark to 1 to avoid buffering
    sock.set(zmq::sockopt::sndhwm, 1000);
    sock.bind("tcp://127.0.0.1:31415");
    int64_t msgCount = 0;
    const auto start = chrono::duration_cast<chrono::microseconds>(
        chrono::high_resolution_clock::now().time_since_epoch()).count();
    while (!ev_flag) {
        const auto delayUs = 1;
        ++msgCount;
        const int64_t pusherTs = chrono::duration_cast<chrono::microseconds>(
            chrono::high_resolution_clock::now().time_since_epoch()).count();
        zmq::message_t msg(&pusherTs, sizeof(pusherTs));
        sock.send(msg, zmq::send_flags::none);
        if (msgCount % 100'0 == 0)
            std::print("Sent {} messages, {} msg/s\n", msgCount,
                       1'000'000 * msgCount / (pusherTs - start) );

        //std::this_thread::sleep_for(std::chrono::microseconds(delayUs));
    }
    std::print("Pusher exited gracefully\n");
    return 0;
}
