#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#include <intrin.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "safe99_common/assert.h"
#include "safe99_common/defines.h"

// 매크로 함수
// -----------------------------------------------------------------

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) > 0) ? (a) : -(a))
#define ROUND_INT(a) ((int)((a) + 0.5f))

// -----------------------------------------------------------------

START_EXTERN_C

SAFE99_API FORCEINLINE bool log2int64(uint32_t* p_out_index, const uint64_t num)
{
    ASSERT(p_out_index != NULL, "p_out_index == NULL");

#ifdef _MSC_VER
    #ifdef _M_X64
        return _BitScanReverse64((unsigned long*)p_out_index, num);
    #else
        uint32_t low32 = (uint32_t)num;
        uint32_t high32 = num >> 32;

        if (high32 > 0)
        {
            _BitScanReverse((unsigned long*)p_out_index, high32);
            *p_out_index += 32;
            return true;
        } else
        {
            return _BitScanReverse((unsigned long*)p_out_index, low32);
        }
    #endif // _M_X64
#endif // _MSC_VER
}

END_EXTERN_C

#endif // MATH_UTIL_H