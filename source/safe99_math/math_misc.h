//***************************************************************************
// 
// 파일: math_misc.h
// 
// 설명: 기타 수학 함수
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/11/20
// 
//***************************************************************************

#ifndef SAFE99_MATH_MISC_H
#define SAFE99_MATH_MISC_H

#include "safe99_common/defines.h"
#include "safe99_math/math_defines.h"

START_EXTERN_C

SAFE99_API int __stdcall log2int64(const uint64_t num);

// DirectXMath에서 가져옴
FORCEINLINE void __stdcall get_sin_cos(const float rad, float* p_out_sin, float* p_out_cos)
{
    ASSERT(p_out_sin != NULL, "p_out_sin == NULL");
    ASSERT(p_out_cos != NULL, "p_out_cos == NULL");

    // Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
    float quotient = (float)PI_1_DIV_2PI * rad;
    if (rad >= 0.0f)
    {
        quotient = (float)((int)(quotient + 0.5f));
    }
    else
    {
        quotient = (float)((int)(quotient - 0.5f));
    }
    float y = rad - (float)TWO_PI * quotient;

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
    *p_out_sin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

    // 10-degree minimax approximation
    const float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
    *p_out_cos = sign * p;
}

END_EXTERN_C

#endif // SAFE99_MATH_MISC_H