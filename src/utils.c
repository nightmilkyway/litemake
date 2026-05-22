#include <stdio.h>
#include <string.h>

#include "error.h"
#include "utils.h"

void term_memcpy(void *dest, const void *src, size_t n) {
	if(!dest) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	if(!src) {litemake_error(LITEMAKE_ERR_NULL_POINTER, "null pointer encountered");}
	memcpy(dest, src, n);
}
