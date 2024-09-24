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

f32 _normalize_sinf(f32 x)
{
    x *= +0.1591549430919f;
    x -= floorf(x + 0.5f);
    x *= +6.2831853071796f;
    return x;
}

f32 _normalize_cosf(f32 x)
{
    x *= +0.1591549430919f;
    x -= floorf(x + 0.75f) - 0.25f;
    x *= +6.2831853071796f;
    return x;
}

f32 _faster_unnormedf(f32 x)
{
    x += -0.3183098861838f * x * __builtin_fabsf(x);
    x += +0.3451140202480f * x * __builtin_fabsf(x);
    return x;
}

inline f32 sinf(f32 x) { return _faster_unnormedf(_normalize_sinf(x)); }
inline f32 cosf(f32 x) { return _faster_unnormedf(_normalize_cosf(x)); }
