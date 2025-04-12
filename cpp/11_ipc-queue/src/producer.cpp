#include "test_payload.pb.h"

#include <boost/interprocess/ipc/message_queue.hpp>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>

using namespace boost::interprocess;

int main() {
    spdlog::init_thread_pool(64, 1);
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto lgr = std::make_shared<spdlog::async_logger>("", stdout_sink, spdlog::thread_pool(),
                                                      spdlog::async_overflow_policy::overrun_oldest);
    lgr->set_pattern("%Y-%m-%dT%T.%f|%5t|%8l| %v");
    lgr->info("consumer started");

    try {
        // Erase previous message queue
        message_queue::remove("my_message_queue");
        uint32_t seq_num = 0;
        // Create a message_queue.
        message_queue mq(create_only, "my_message_queue", 100, 128);

        long long prev_unix_time_us = 0;
        constexpr long intervals_us = 5'000'000;
        uint32_t prev_seq_num = 0;

        while (true) {
            boost_ipc_payload::TestMessage msg;
            auto now = std::chrono::system_clock::now();
            // use long long, not long, Windows uses long long, Linux use long
            const long long unix_time_us =
                    std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
            msg.set_unix_time_us(unix_time_us);
            msg.set_seq_num(seq_num++);
            msg.set_is_active(seq_num % 2 == 0);
            auto buf = msg.SerializeAsString();
            mq.send(buf.data(), buf.size(), 0);
            if (unix_time_us - prev_unix_time_us > intervals_us) {
                lgr->info("{} msg/sec sent. Latest msg: seq_num={}, timestamp_us={}",
                          (seq_num - prev_seq_num) / ((unix_time_us - prev_unix_time_us) / 1'000'000), msg.seq_num(),
                          msg.unix_time_us());
                prev_seq_num = seq_num;
                prev_unix_time_us = unix_time_us;
            }
        }
    } catch (interprocess_exception &ex) {
        std::cout << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
