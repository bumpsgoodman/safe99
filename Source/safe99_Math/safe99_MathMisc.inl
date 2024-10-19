// 작성자: bumpsgoodman
// 작성일: 2024-08-24

#ifndef SAFE99_MATH_MISC_H
#define SAFE99_MATH_MISC_H

#include "safe99_Common/Common.h"
#include "safe99_MathDefine.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) (((a) > 0) ? (a) : -(a))

#define ROUND(x) ((x) >= 0.0f ? (float)((int)((x) + 0.5f)) : (float)((int)((x) - 0.5f)))
#define FLOOR(x) ((x) >= 0.0f ? (float)((int)(x)) : (float)((int)((x) - 1.0f)))
#define TRUNC(x) ((x) >= 0.0f ? (float)((int)(x)) : (float)((int)(x)))
#define ROUND_INT(x) ((x) >= 0.0f ? (int)((x) + 0.5f) : (int)((x) - 0.5f))
#define FLOOR_INT(x) ((x) >= 0.0f ? (int)(x) : (int)((x) - 1.0f))
#define TRUNC_INT(x) ((x) >= 0.0f ? (int)(x) : (int)(x))

extern
#ifdef __cplusplus
"C" {
#endif // __cplusplus

SAFE99_GLOBAL_FUNC int __stdcall Log2Int64(const uint64_t num);

#ifdef __cplusplus
}
#endif // __cplusplus

inline float __stdcall Mod(const float a, const float b)
{
    return a - b * TRUNC(a / b);
}

inline float __stdcall Clamp(const float value, const float minValue, const float maxValue)
{
    return MAX(MIN(value, maxValue), minValue);
}

inline float __stdcall Wrap(const float value, const float minValue, const float maxValue)
{
    if (value > maxValue)
    {
        return minValue + Mod(value - maxValue, maxValue);
    }
    else if (value < minValue)
    {
        return maxValue + Mod(minValue + value, maxValue);
    }

    return value;
}

inline float __stdcall DegreeToRad(const float degree)
{
    return degree * PI_DIV_180;
}

// DirectXMath에서 가져옴
inline void __stdcall GetSinCos(const float rad, float* pOutSin, float* pOutCos)
{
    ASSERT(pOutSin != NULL, "pOutSin == NULL");
    ASSERT(pOutCos != NULL, "pOutCos == NULL");

    // Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
    float quotient = PI_1_DIV_2PI * rad;
    if (rad >= 0.0f)
    {
        quotient = (float)((int)(quotient + 0.5f));
    }
    else
    {
        quotient = (float)((int)(quotient - 0.5f));
    }
    float y = rad - TWO_PI * quotient;

    // Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
    float sign;
    if (y > (float)PI_DIV_2)
    {
        y = (float)PI - y;
        sign = -1.0f;
    }
    else if (y < -(float)PI_DIV_2)
    {
        y = -(float)PI - y;
        sign = -1.0f;
    }
    else
    {
        sign = +1.0f;
    }

    const float y2 = y * y;

    // 11-degree minimax approximation
    *pOutSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

    // 10-degree minimax approximation
    const float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
    *pOutCos = sign * p;
}

#endif // SAFE99_MATH_MISC_H