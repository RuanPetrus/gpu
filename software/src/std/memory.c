#include "memory.h"
// TODO: Optimize
void *memset(void *dst0, i32 c, u32 n) __attribute__((optimize("-fno-tree-loop-distribute-patterns")));
void *memset(void *dst0, i32 c, u32 n) 
{
	u8 *dst = dst0;
	for (u32 i = 0; i < n; i++) dst[i] = c;
	return dst0;
}

// TODO: Optimize, 
// TODO: Handle overlap
// NOTE: take a look at https://opensource.apple.com/source/xnu/xnu-2050.7.9/libsyscall/wrappers/memcpy.c
void *memcpy(void *dst0, const void *src0, u32 n) __attribute__((optimize("-fno-tree-loop-distribute-patterns")));
void *memcpy(void *dst0, const void *src0, u32 n)
{
	u8 *dst = dst0;
	const u8 *src = src0;
	for (u32 i = 0; i < n; i++) dst[i] = src[i];
	return dst0;
}

void *memmove(void *dst0, const void *src0, u32 n) __attribute__((optimize("-fno-tree-loop-distribute-patterns")));
void *memmove(void *dst0, const void *src0, u32 n)
{
	return memcpy(dst0, src0, n);
}

// TODO: Optimize
i32 memcmp(const void *vl, const void *vr, u32 n) __attribute__((optimize("-fno-tree-loop-distribute-patterns")));
i32 memcmp(const void *vl, const void *vr, u32 n)
{
    const u8 *l=vl, *r=vr;
	while(n && *l == *r) n--, l++, r++;
    return n ? *l-*r : 0;
}
