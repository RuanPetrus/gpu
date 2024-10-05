#include "math.h"

inline f32 clamp(f32 val, f32 l, f32 r)
{
	return (val < l ? l : (val > r ? r : val));
}

inline i32 clampi32(i32 val, i32 l, i32 r)
{
	return (val < l ? l : (val > r ? r : val));
}

inline u32 nbits_mask(u32 num_bytes)
{
	return (1 << (num_bytes)) - 1;
}

//TODO: Better floorf implementation
f32 floorf(f32 x)
{
	return (f32) (i32) x;
}

f32 sinf(f32 x)
{
	return cosf(PI/2.0f - x);
}
#define COS_NUMBER_ITERATION 6
f32 cosf(f32 x)
{
	i32 div = (i32)(x/PI);
	x = x - (div * PI);
	i8 sign = 1;
	if (div % 2 != 0)
		sign = -1;

	f32 result = 1.0f;
    f32 inter = 1.0f;
    f32 num = x * x;
    for (int i = 1; i <= COS_NUMBER_ITERATION; i++)
    {
        f32 comp = 2.0f * i;
        f32 den = comp * (comp - 1.0f);
        inter *= num / den;
        if (i % 2 == 0)
            result += inter;
        else
            result -= inter;
    }
	return sign * result;
}
