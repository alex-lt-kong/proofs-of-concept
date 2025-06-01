# Singleton

## Performance

- `Intel(R) Core(TM) i7-14700` +
  `Microsoft (R) C/C++ Optimizing Compiler Version 19.43.34810 for x64`

```
Run on (28 X 2112 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x14)
  L1 Instruction 32 KiB (x14)
  L2 Unified 2048 KiB (x14)
  L3 Unified 33792 KiB (x1)
-----------------------------------------------------------------
Benchmark                       Time             CPU   Iterations
-----------------------------------------------------------------
BM_MySingletonMeyers          304 ns          308 ns      2635294
BM_MySingletonMutex         21711 ns        21310 ns        34462
BM_MySingletonCallOnce       5539 ns         5625 ns       100000
```

- `AMD Ryzen 5 PRO 6650U` + `gcc 14.2.0`

```
Run on (28 X 2112 MHz CPU s)
CPU Caches:
L1 Data 48 KiB (x14)
L1 Instruction 32 KiB (x14)
L2 Unified 2048 KiB (x14)
L3 Unified 33792 KiB (x1)
-----------------------------------------------------------------
Benchmark                       Time             CPU   Iterations
-----------------------------------------------------------------
BM_MySingletonMeyers          304 ns          308 ns      2635294
BM_MySingletonMutex         21711 ns        21310 ns        34462
BM_MySingletonCallOnce       5539 ns         5625 ns       100000
```
