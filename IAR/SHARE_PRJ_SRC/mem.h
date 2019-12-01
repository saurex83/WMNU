#include "stdint.h"
#include "stdlib.h"

void re_free(void* ptr);
void* re_malloc(size_t size);
uint16_t heap_ptr(size_t size);
void re_memcpy(void *dst, const void *src, size_t n);