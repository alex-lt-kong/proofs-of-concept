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

- `Cortex-A53` + `gcc 12.2.0`

```
Run on (4 X 1400 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 512 KiB (x1)
Load Average: 0.81, 2.33, 1.74
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
-----------------------------------------------------------------
Benchmark                       Time             CPU   Iterations
-----------------------------------------------------------------
BM_MySingletonMeyers         1467 ns         1467 ns       397656
BM_MySingletonMutex        113541 ns       113480 ns         6142
BM_MySingletonCallOnce      27693 ns        27681 ns        25330
```