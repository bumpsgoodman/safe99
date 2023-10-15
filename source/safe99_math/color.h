//***************************************************************************
// 
// 파일: color.h
// 
// 설명: float4 색상
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/10/06
// 
//***************************************************************************

#ifndef COLOR_H
#define COLOR_H

#include "safe99_common/defines.h"
#include "safe99_math/math_misc.h"

START_EXTERN_C

FORCEINLINE color_t __stdcall color_set(const float r, const float g, const float b, const float a)
{
    /*
    const color_t color =
    {
        MAX(0.0f, MIN(1.0f, r)),
        MAX(0.0f, MIN(1.0f, g)),
        MAX(0.0f, MIN(1.0f, b)),
        MAX(0.0f, MIN(1.0f, a))
    };
    */
    
    const color_t color =
    {
        r, g, b, a
    };
    return color;
}

FORCEINLINE uint32_t __stdcall color_to_argb(const color_t color)
{
    const uint32_t argb = ((uint32_t)(color.a * 255.0f) << 24)
        | (uint32_t)(color.r * 255.0f) << 16
        | (uint32_t)(color.g * 255.0f) << 8
        | (uint32_t)(color.b * 255.0f);
    return argb;
}

END_EXTERN_C

#endif // COLOR_H