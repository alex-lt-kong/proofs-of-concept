#include "pc_queue.h"
#include "pc_queue_impl/atomic_queue.h"
#include "pc_queue_impl/mutex_deque.h"
// #include "pc_queue_impl/ramalhete_queue.h"
#include "pc_queue_impl/my_ringbuffer.h"
#include "pc_queue_impl/reader_writer_queue.h"
#include <chrono>
#include <iostream>
#include <print>
#include <unistd.h>

using namespace std;

void cb_on_dequeue(const uint32_t &ele, std::size_t msg_count) {
  std::println("{},{}", ele, msg_count);
}

template <class T_QUEUE>
auto benchmark(size_t iterations, uint32_t *elements, size_t element_count) {
  auto start = chrono::high_resolution_clock::now();

  auto pcq = PcQueue<T_QUEUE>(iterations, cb_on_dequeue);
  pcq.start();
  start = chrono::high_resolution_clock::now();
  for (uint32_t i = 0; i < iterations; i++) {
    pcq.enqueue(elements[i % element_count]);
  }
  pcq.wait();
  const auto end = chrono::high_resolution_clock::now();

  return std::make_tuple(
      pcq.handled_msg_count(),
      chrono::duration_cast<chrono::nanoseconds>(end - start).count());
}

template <class T_QUEUE> void benchmark_executor(string impl_name) {
  constexpr size_t iter_count = 10;
  uint32_t ele_arr[] = {0, 2, 2, 2, 4, 5, 5, 7, 8, 9};
  constexpr size_t ele_len = sizeof(ele_arr) / sizeof(ele_arr[0]);

  cout << "===== " << impl_name << " =====\n";

  for (size_t i = 0; i < 10; ++i) {
    uint32_t ele_counter[ele_len] = {0};
    auto [msg_count, elapsed_ns] =
        benchmark<T_QUEUE>(iter_count, ele_arr, ele_len);
    std::locale loc("");
    std::cout.imbue(loc);
    cout << "iter: " << i << ", elapsed_ms: " << elapsed_ns / 1000 / 1000
         << ", handled_msg: " << msg_count
         << ", ops/sec: " << msg_count * 1000 * 1000 * 1000 / elapsed_ns;
    cout << "\n";
  }
  std::cout << "\n" << std::endl;
}

void print_cpu_model() {
  char buffer[PATH_MAX];
  FILE *fp = fopen("/proc/cpuinfo", "r");

  if (fp == NULL) {
    perror("Failed to open /proc/cpuinfo");
    return;
  }

  while (fgets(buffer, PATH_MAX, fp) != NULL) {
    if (strncmp(buffer, "model name", 10) == 0) {
      char *model_name = strchr(buffer, ':');
      if (model_name != NULL) {
        printf("CPU Model: %s\n\n", model_name + 2); // Skip the colon and space
        break;
      }
    }
  }
  fclose(fp);
}

int main(void) {
  print_cpu_model();

  benchmark_executor<aqb>("max0x7ba/OptimistAtomicQueues");
  benchmark_executor<myrb>("PoC::LockFree::RingBufferSPSC");
  benchmark_executor<rwq>("moodycamel::ReaderWriterQueue");
  benchmark_executor<dq>("std::deque with std::mutex");
  return 0;
}
