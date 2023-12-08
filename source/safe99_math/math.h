//***************************************************************************
// 
// 파일: math.h
// 
// 설명: 수학 함수
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/09/25
// 
//***************************************************************************

#ifndef SAFE99_MATH_H
#define SAFE99_MATH_H

#include <math.h>

#include "safe99_common/defines.h"
#include "color.h"
#include "math_defines.h"
#include "math_misc.h"
#include "matrix.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) (((a) > 0) ? (a) : -(a))

#define ROUND(x) ((x) >= 0.0f ? (float)((int)((x) + 0.5f)) : (float)((int)((x) - 0.5f)))
#define FLOOR(x) ((x) >= 0.0f ? (float)((int)(x)) : (float)((int)((x) - 1.0f)))
#define TRUNC(x) ((x) >= 0.0f ? (float)((int)(x)) : (float)((int)(x)))
#define ROUND_INT(x) ((x) >= 0.0f ? (int)((x) + 0.5f) : (int)((x) - 0.5f))
#define FLOOR_INT(x) ((x) >= 0.0f ? (int)(x) : (int)((x) - 1.0f))
#define TRUNC_INT(x) ((x) >= 0.0f ? (int)(x) : (int)(x))

FORCEINLINE float __stdcall mod(const float a, const float b)
{
    return a - b * TRUNC(a / b);
}

FORCEINLINE float __stdcall clamp(const float value, const float min_value, const float max_value)
{
    return MAX(MIN(value, max_value), min_value);
}

FORCEINLINE float __stdcall wrap(const float value, const float min_value, const float max_value)
{
    if (value > max_value)
    {
        return min_value + mod(value - max_value, max_value);
    }
    else if (value < min_value)
    {
        return max_value + mod(min_value + value, max_value);
    }

    return value;
}

#endif // SAFE99_MATH_H