#include "library.h"

#include <string.h>
#include <stdio.h>

void int64_to_char_v1(int64_t in, char *buffer) {
    if (in == 0) {
        buffer[0] = '0';
        return;
    }

    int idx = 0;
    //int64_t _in = in > 0 ? in : -in;

    int is_negative = (in < 0);
    // Introduction of uint64_t is important, as -1 * INT64_MIN > INT64_MAX!
    uint64_t abs_in = is_negative ? ((uint64_t) (-(in + 1)) + 1)
                                  : (uint64_t) in;
    while (abs_in > 0) {
        buffer[idx++] = abs_in % 10 + '0';
        abs_in /= 10;
    }
    if (in < 0)
        buffer[idx++] = '-';
    size_t start = 0, end = idx - 1;
    while (start < end) {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }
}


void int64_to_char_v2(int64_t in, char *buffer) {
    if (in == 0) {
        buffer[0] = '0';
        return;
    }

    int is_negative = (in < 0);
    // Introduction of uint64_t is important, as -1 * INT64_MIN > INT64_MAX!
    uint64_t abs_in = is_negative ? ((uint64_t) (-(in + 1)) + 1)
                                  : (uint64_t) in;
    size_t digit = 1;
    if (abs_in >= 1000000000000000000)
        digit = 19;
    else if (abs_in >= 100000000000000000)
        digit = 18;
    else if (abs_in >= 10000000000000000)
        digit = 17;
    else if (abs_in >= 1000000000000000)
        digit = 16;
    else if (abs_in >= 100000000000000)
        digit = 15;
    else if (abs_in >= 10000000000000)
        digit = 14;
    else if (abs_in >= 1000000000000)
        digit = 13;
    else if (abs_in >= 100000000000)
        digit = 12;
    else if (abs_in >= 10000000000)
        digit = 11;
    else if (abs_in >= 1000000000)
        digit = 10;
    else if (abs_in >= 100000000)
        digit = 9;
    else if (abs_in >= 10000000)
        digit = 8;
    else if (abs_in >= 1000000)
        digit = 7;
    else if (abs_in >= 100000)
        digit = 6;
    else if (abs_in >= 10000)
        digit = 5;
    else if (abs_in >= 1000)
        digit = 4;
    else if (abs_in >= 100)
        digit = 3;
    else if (abs_in >= 10)
        digit = 2;

    if (is_negative) {
        digit++;
        buffer[0] = '-';
    }

    while (digit > 0 + is_negative) {
        buffer[digit - 1] = abs_in % 10 + '0';
        --digit;
        abs_in /= 10;
    }
}