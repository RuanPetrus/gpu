#include "defs.h"

#ifndef MEMORY_H
#define MEMORY_H

void *memset(void *dst0, i32 c, u32 n);
void *memcpy(void *dst0, const void *src0, u32 n);
void *memmove(void *dst0, const void *src0, u32 n);
i32 memcmp(const void *vl, const void *vr, u32 n);

#endif // MEMORY_H
