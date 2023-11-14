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

#define GET_ALPHA(color) ((uint8_t)((color).a * 255.0f))
#define GET_RED(color) ((uint8_t)((color).r * 255.0f))
#define GET_GREEN(color) ((uint8_t)((color).g * 255.0f))
#define GET_BLUE(color) ((uint8_t)((color).b * 255.0f))

#define GET_ALPHA_ARGB(argb) (((argb) & 0xff000000) >> 24)
#define GET_RED_ARGB(argb) (((argb) & 0x00ff0000) >> 16)
#define GET_GREEN_ARGB(argb) (((argb) & 0x0000ff00) >> 8)
#define GET_BLUE_ARGB(argb) ((argb) & 0x000000ff)

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
    const uint32_t argb = (GET_ALPHA(color) << 24)
        | GET_RED(color) << 16
        | GET_GREEN(color) << 8
        | GET_BLUE(color);
    return argb;
}

FORCEINLINE color_t __stdcall argb_to_color(const uint32_t argb)
{
    return color_set((float)(GET_RED_ARGB(argb) / 255.0f),
                     (float)(GET_GREEN_ARGB(argb) / 255.0f),
                     (float)(GET_BLUE_ARGB(argb) / 255.0f),
                     (float)(GET_ALPHA_ARGB(argb) / 255.0f));
}

END_EXTERN_C

#endif // COLOR_H