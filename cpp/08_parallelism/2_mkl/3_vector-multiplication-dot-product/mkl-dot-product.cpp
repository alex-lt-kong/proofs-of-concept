#include "../../utils.h"
#include "func.h"
#include "rand.h"

#include <ios>
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
    sum = cblas_ddot(arr_size, vec_a.data(), 1, vec_b.data(), 1);
    // mkl_dot_product(vec_a.data(), vec_b.data(), &sum, arr_size);
    auto t1 = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(t1 - t0);
    cout << setw(15) << setprecision(5) << sum << ", " << setw(10)
         << setprecision(2) << duration.count() / 1000.0 << endl;
  }
}
