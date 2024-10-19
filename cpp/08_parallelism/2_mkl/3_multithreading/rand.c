#include <stdlib.h>
#include <time.h>

int get_rand_max() { return RAND_MAX; };

void init_random() { srand(time(NULL) / (3600 * 24)); }

int get_random_int() { return rand(); }

double get_random_0_to_1() { return (double)rand() / (RAND_MAX + 1.0); }
