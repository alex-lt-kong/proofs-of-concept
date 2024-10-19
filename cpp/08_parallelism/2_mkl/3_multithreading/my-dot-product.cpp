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

static int cpu_count = -1;

typedef void (*MathFunc)(const double *, const double *, size_t, long,
                         double *);

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

void element_wise_pow_job(const double *vec_a, const double *vec_b,
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
    *sum += pow(vec_a[i + offset], vec_b[i + offset]);
  }
}

void element_wise_log_job(const double *vec_a, const double *vec_b,
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
    *sum += log(vec_a[i + offset]) / log(vec_b[i + offset]);
  }
}

void query_cpu_count() {
  if (cpu_count == -1)
    cpu_count = thread::hardware_concurrency();
  if (cpu_count <= 0) {
    cpu_count = 8;
    cerr << "thread::hardware_concurrency() failed, default to " << cpu_count
         << "\n";
  }
}

double jobs_dispatcher(MathFunc job_func, const double *vec_a,
                       const double *vec_b, int64_t arr_size) {
  double sum = 0;

  int thread_count = ceil(arr_size / 1000.0 / 1000.0);
  query_cpu_count();
  thread_count = thread_count > cpu_count ? cpu_count : thread_count;
  unique_ptr<double[]> sums(new double[thread_count + 1]);
  vector<thread> threads;
  threads.reserve(thread_count);
  for (int i = 0; i < thread_count; ++i) {
    sums[i] = 0;
    threads.emplace_back(thread(job_func, (vec_a), (vec_b),
                                arr_size / thread_count,
                                arr_size / thread_count * i, &sums[i]));
  }

  // handle quotient
  sums[thread_count] = 0;
  if (arr_size % thread_count != 0) {
    threads.emplace_back(thread(job_func, vec_a, vec_b, arr_size % thread_count,
                                arr_size / thread_count * thread_count,
                                &sums[thread_count]));
  }

  for (size_t i = 0; i < threads.size(); ++i) {
    threads[i].join();
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
  cout << "exp,vector_size,result,takes(ms),result,takes(ms)\n";
  for (int e = 0; e < exp; ++e) {
    size_t arr_size = pow(base, e);
    cout << fixed << setw(3) << e << ", " << setw(15) << arr_size << ", ";
    unique_ptr<double[]> vec_a(new double[arr_size]);
    unique_ptr<double[]> vec_b(new double[arr_size]);
    for (size_t i = 0; i < arr_size; ++i) {
      vec_a[i] = get_random_0_to_1();
      vec_b[i] = get_random_0_to_1();
    }

    auto t0 = chrono::steady_clock::now();
    auto sum =
        jobs_dispatcher(dot_product_job, vec_a.get(), vec_b.get(), arr_size);
    auto t1 = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(t1 - t0);
    cout << setw(15) << setprecision(5) << sum << ", " << setw(10)
         << setprecision(2) << duration.count() / 1000.0;

    t0 = chrono::steady_clock::now();
    sum = jobs_dispatcher(element_wise_pow_job, vec_a.get(), vec_b.get(),
                          arr_size);
    t1 = chrono::steady_clock::now();
    duration = chrono::duration_cast<chrono::microseconds>(t1 - t0);
    cout << setw(15) << setprecision(5) << sum << ", " << setw(10)
         << setprecision(2) << duration.count() / 1000.0;

    t0 = chrono::steady_clock::now();
    sum = jobs_dispatcher(element_wise_log_job, vec_a.get(), vec_b.get(),
                          arr_size);
    t1 = chrono::steady_clock::now();
    duration = chrono::duration_cast<chrono::microseconds>(t1 - t0);
    cout << setw(20) << setprecision(5) << sum << ", " << setw(10)
         << setprecision(2) << duration.count() / 1000.0 << endl;
  }
  return 0;
}