#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
__declspec(dllexport)
#endif
int get_rand_max() { return RAND_MAX; };

#ifdef _WIN32
__declspec(dllexport)
#endif
void init_random() { srand(time(NULL) / (3600 * 24)); }

#ifdef _WIN32
__declspec(dllexport)
#endif
int get_random_int() { return rand(); }

#ifdef _WIN32
__declspec(dllexport)
#endif
double get_random_0_to_1() { return (double)rand() / (RAND_MAX + 1.0); }
