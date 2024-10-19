from ctypes import c_double, CDLL

import ctypes
import numpy as np
import sys
import time

if sys.platform == 'win32':
  lib_file = "./build/Release/func.dll"
else:
  lib_file = "./build/librand.so"
func = CDLL(lib_file)
func.get_random_0_to_1.restype = c_double
func.init_random()

exp = 0
if len(sys.argv) > 1:
  exp = int(sys.argv[1])
if exp <= 0:
    exp = 5

base = 2
print("exp,vector_size,result,takes(ms),result,takes(ms)")
for e in range(exp):
  arr_size = base ** e
  print(f"{e:3},{arr_size:15}, ", end='')
  vec_a = np.empty((arr_size,))
  vec_b = np.empty((arr_size,))
  for i in range(arr_size):
    vec_a[i] = func.get_random_0_to_1()
    vec_b[i] = func.get_random_0_to_1()
  
  t0 = time.time()
  result = np.dot(vec_a, vec_b)
  t1 = time.time()
  print(f'{result:15.5f}, {(t1 - t0) * 1000:10,.2f}', end='')

  t0 = time.time()
  result = np.power(vec_a, vec_b).sum()
  t1 = time.time()
  print(f'{result:15.5f}, {(t1 - t0) * 1000:10,.2f}', end='')

  t0 = time.time()
  result = (np.log(vec_a) / np.log(vec_b)).sum()
  t1 = time.time()    
  print(f'{result:20.5f}, {(t1 - t0) * 1000:10,.2f}')
  

