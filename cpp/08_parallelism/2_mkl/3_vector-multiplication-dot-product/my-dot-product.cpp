#include "../../utils.h"
#include "func.h"
#include "rand.h"

#include <iomanip>
#include <iostream>
#include <math.h>
#include <memory>
#include <thread>
#include <vector>

using namespace std;

void dot_product_job(const double *vec_a, const double *vec_b,
                     size_t block_size, long offset, double *sum) {
#if defined(__INTEL_COMPILER)
#pragma ivdep
// Pragmas are specific for the compiler and platform in use. So the best bet is
// to look at compiler's documentation.
// https://stackoverflow.com/questions/5078679/what-is-the-scope-of-a-pragma-directive
#elif defined(__GNUC__)
#pragma GCC ivdep
#endif
  for (size_t i = 0; i < block_size; ++i) {
    *sum += vec_a[i + offset] * vec_b[i + offset];
  }
}

double jobs_dispatcher(const double *vec_a, const double *vec_b,
                       int64_t arr_size) {
  double sum = 0;
  vector<thread> threads;
  // vector<double> sums;
  size_t thread_count = arr_size / (1000 * 1000);
  size_t cpu_count = thread::hardware_concurrency();
  if (cpu_count <= 0) {
    cpu_count = 8;
    cerr << "thread::hardware_concurrency() failed, default to " << cpu_count
         << "\n";
  }
  thread_count = thread_count > cpu_count ? cpu_count : thread_count;
  if (thread_count == 0)
    thread_count = 1;
  double sums[thread_count + 1];
  for (size_t i = 0; i < thread_count; ++i) {
    sums[i] = 0;
    threads.emplace_back(thread(dot_product_job, (vec_a), (vec_b),
                                arr_size / thread_count,
                                arr_size / thread_count * i, &sums[i]));
  }

  if (arr_size % thread_count != 0) {
    sums[thread_count] = 0;
    threads.emplace_back(
        thread(dot_product_job, vec_a, vec_b, arr_size % thread_count,
               arr_size / thread_count * thread_count, &sums[thread_count]));
    threads[thread_count].join();
    sum += sums[thread_count];
  }

  for (size_t i = 0; i < thread_count; ++i) {
    threads[i].join();
  }
  for (size_t i = 0; i < thread_count; ++i) {
    sum += sums[i];
  }
  return sum;
}

int main(int argc, char *argv[]) {
  init_random();
  float base = 2;
  int exp = 0;
  if (argc == 2)
    exp = atoi(argv[1]);
  if (exp <= 0)
    exp = 5;
  cout << "exp,vector_size,result,takes(ms)\n";
  for (int e = 0; e < exp; ++e) {
    long arr_size = pow(base, e);
    cout << fixed << setw(3) << e << ", " << setw(15) << arr_size << ", ";
    vector<double> vec_a;
    vector<double> vec_b;
    for (long i = 0; i < arr_size; ++i) {
      vec_a.push_back(get_random_0_to_1());
    }
    for (long i = 0; i < arr_size; ++i) {
      vec_b.push_back(get_random_0_to_1());
    }

    auto t0 = chrono::steady_clock::now();
    auto sum = jobs_dispatcher(vec_a.data(), vec_b.data(), arr_size);
    auto t1 = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(t1 - t0);
    cout << setw(15) << setprecision(5) << sum << ", " << setw(10)
         << setprecision(2) << duration.count() / 1000.0 << endl;
  }
  return 0;
}