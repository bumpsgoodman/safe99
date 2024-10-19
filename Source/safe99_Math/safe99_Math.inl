// 작성자: bumpsgoodman
// 작성일: 2024-08-24

#ifndef SAFE99_MATH_H
#define SAFE99_MATH_H

#include "safe99_MathMisc.inl"

#include <immintrin.h>
#include <intrin.h>

typedef ALIGN16 union COLORF
{
    __m128 SSE;
    float RGBA[4];
    struct
    {
        float Red;
        float Green;
        float Blue;
        float Alpha;
    };
} COLORF;

typedef ALIGN16 union VECTOR2_INT
{
    __m128i SSE;
    int     XY[2];
    struct
    {
        int X;
        int Y;
    };
} VECTOR2_INT;

static const VECTOR2_INT s_vector2_int_zero = { 0, 0, 0, 0 };
static const VECTOR2_INT s_vector2_int_one = { 0, 0, 0, 0 };

inline VECTOR2_INT __vectorcall Vector2IntSet(const int x, const int y)
{
    const VECTOR2_INT result = { _mm_set_epi32(0, 0, y, x) };
    return result;
}

inline VECTOR2_INT __vectorcall Vector2IntSet1(const int x)
{
    const VECTOR2_INT result = { _mm_set_epi32(0, 0, x, x) };
    return result;
}

inline VECTOR2_INT __vectorcall Vector2IntAdd(const VECTOR2_INT v0, const VECTOR2_INT v1)
{
    const VECTOR2_INT result = { _mm_add_epi32(v0.SSE, v1.SSE) };
    return result;
}

inline VECTOR2_INT __vectorcall Vector2IntSub(const VECTOR2_INT v0, const VECTOR2_INT v1)
{
    const VECTOR2_INT result = { _mm_sub_epi32(v0.SSE, v1.SSE) };
    return result;
}

inline VECTOR2_INT __vectorcall Vector2IntMul(const VECTOR2_INT v0, const VECTOR2_INT v1)
{
    const VECTOR2_INT result = { _mm_mul_epi32(v0.SSE, v1.SSE) };
    return result;
}

inline VECTOR2_INT __vectorcall Vector2IntDiv(const VECTOR2_INT v0, const VECTOR2_INT v1)
{
    const VECTOR2_INT result = { _mm_div_epi32(v0.SSE, v1.SSE) };
    return result;
}

inline VECTOR2_INT __vectorcall Vector2IntZero()
{
    return s_vector2_int_zero;
}

inline VECTOR2_INT __vectorcall Vector2IntOne()
{
    return s_vector2_int_one;
}

inline VECTOR2_INT __vectorcall Vector2IntMax(const VECTOR2_INT v0, const VECTOR2_INT v1)
{
    VECTOR2_INT result;
    result.SSE = _mm_max_epi32(v0.SSE, v1.SSE);
    return result;
}

inline VECTOR2_INT __vectorcall Vector2IntMin(const VECTOR2_INT v0, const VECTOR2_INT v1)
{
    VECTOR2_INT result;
    result.SSE = _mm_min_epi32(v0.SSE, v1.SSE);
    return result;
}

#endif // SAFE99_MATH_H