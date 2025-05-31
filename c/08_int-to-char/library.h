#ifndef INC_08_INT_TO_CHAR_LIBRARY_H
#define INC_08_INT_TO_CHAR_LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// 1 for negative sign, 19 for INT64_MAX and 1 for \0
static const int BUFFER_SIZE = 1 + 19 + 1;

void int64_to_char_v1(int64_t in, char *buffer);

void int64_to_char_v2(int64_t in, char *buffer);

void int64_to_char_v3(int64_t in, char *buffer);

#ifdef __cplusplus
}
#endif

#endif //INC_08_INT_TO_CHAR_LIBRARY_H
