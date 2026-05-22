#ifndef LITEMAKE_UTILS_H
#define LITEMAKE_UTILS_H

#include <stdio.h>

#define ARRAYSIZE(n) (sizeof(n)/sizeof(n[0]))
#define LITEMAKE_LINESTAT 32

void term_memcpy(void *dest, const void *src, size_t n);

#endif
