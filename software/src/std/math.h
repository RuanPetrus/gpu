#include "defs.h"

#ifndef MATH_H
#define MATH_H

/*
TODO: Add vector functions
TODO: Add more trig functions
*/

// Clamps val to [l, r] interval
f32 clamp(f32 val, f32 l, f32 r);
i32 clampi32(i32 val, i32 l, i32 r);

// Returns a mask with <num_bits> ones
// nbits_mask(3) = 0b0111
// Maximum 32 bits
u32 nbits_mask(u32 num_bits);

#define PI 3.14159265358979323846f
// x must be between [-PI, PI]
f32 sinf(f32 x);
f32 cosf(f32 x);
f32 floorf(float x);

#endif // MATH_H
