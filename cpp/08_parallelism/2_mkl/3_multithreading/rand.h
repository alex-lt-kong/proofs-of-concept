#ifndef RAND_H
#define RAND_H

extern "C" {
int get_rand_max();

void init_random();

int get_random_int();

double get_random_0_to_1();
}

#endif // RAND_H