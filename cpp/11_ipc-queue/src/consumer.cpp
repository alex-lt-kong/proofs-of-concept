#include "test_payload.pb.h"

#include <boost/interprocess/ipc/message_queue.hpp>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>


using namespace boost::interprocess;

int main() {
    constexpr std::string queue_name = "my_message_queue";
    spdlog::init_thread_pool(64, 1);
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto lgr = std::make_shared<spdlog::async_logger>("", stdout_sink, spdlog::thread_pool(),
                                                      spdlog::async_overflow_policy::overrun_oldest);
    lgr->set_pattern("%Y-%m-%dT%T.%f|%5t|%8l| %v");
    lgr->info("consumer started");
    try {
        message_queue mq(open_only, queue_name.c_str());

        unsigned int priority;
        uint32_t msg_received_count = 0;
        message_queue::size_type recvd_size;
        constexpr long intervals_us = 5'000'000;
        long long prev_unix_time_us = 0;
        boost_ipc_payload::TestMessage msg;
        while (true) {
            ++msg_received_count;
            std::string buffer;
            buffer.resize(1024);
            mq.receive(buffer.data(), buffer.size(), recvd_size, priority);
            auto now = std::chrono::system_clock::now();
            auto unix_time_us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
            msg.ParseFromString(buffer);
            if (unix_time_us - prev_unix_time_us > intervals_us) {
                auto latency_us = unix_time_us - msg.unix_time_us();
                lgr->info("{} msg/sec received. Latest msg: recvd_size={}, seq_num={}, timestamp_us={}, latency(us)={}",
                          msg_received_count / ((unix_time_us - prev_unix_time_us) / 1'000'000), recvd_size,
                          msg.seq_num(), msg.unix_time_us(), latency_us);
                prev_unix_time_us = unix_time_us;
                msg_received_count = 0;
            }
        }
    } catch (interprocess_exception &ex) {
        message_queue::remove(queue_name.c_str());
        lgr->error("Error: {}", ex.what());
        return 1;
    }
    message_queue::remove(queue_name.c_str());
    return 0;
}
