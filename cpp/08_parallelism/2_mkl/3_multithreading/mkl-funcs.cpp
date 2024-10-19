#include "rand.h"

#include <ios>
#include <memory>
#include <mkl.h>

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char *argv[]) {
  init_random();
  double sum;
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
    unique_ptr<double[]> vec_r1(new double[arr_size]);
    unique_ptr<double[]> vec_r2(new double[arr_size]);
    unique_ptr<double[]> vec_r3(new double[arr_size]);
    for (size_t i = 0; i < arr_size; ++i) {
      vec_a[i] = get_random_0_to_1();
      vec_b[i] = get_random_0_to_1();
    }

    auto t0 = chrono::steady_clock::now();
    sum = cblas_ddot(arr_size, vec_a.get(), 1, vec_b.get(), 1);
    auto t1 = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(t1 - t0);
    cout << setw(15) << setprecision(5) << sum << ", " << setw(10)
         << setprecision(2) << duration.count() / 1000.0;

    // hand over CPU to OS for other tasks
    this_thread::sleep_for(chrono::milliseconds(1000));

    t0 = chrono::steady_clock::now();
    vdPow(arr_size, vec_a.get(), vec_b.get(), vec_r1.get());
    sum = cblas_dasum(arr_size, vec_r1.get(), 1);
    t1 = chrono::steady_clock::now();
    duration = chrono::duration_cast<chrono::microseconds>(t1 - t0);
    cout << setw(15) << setprecision(5) << sum << ", " << setw(10)
         << setprecision(2) << duration.count() / 1000.0 << endl;
  }
}
