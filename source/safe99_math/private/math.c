//***************************************************************************
// 
// 파일: math.c
// 
// 설명: 수학 함수
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/08/31
// 
//***************************************************************************

#include "precompiled.h"

#include "math.h"

bool log2int64(uint32_t* p_out_index, const uint64_t num)
{
    ASSERT(p_out_index != NULL, "p_out_index == NULL");

#if (defined(_WIN64))
    return _BitScanReverse64((unsigned long*)p_out_index, num);
#elif (defined(_WIN32))
    uint32_t low32 = (uint32_t)num;
    uint32_t high32 = num >> 32;

    if (high32 > 0)
    {
        _BitScanReverse((unsigned long*)p_out_index, high32);
        *p_out_index += 32;
        return true;
    }
    else
    {
        return _BitScanReverse((unsigned long*)p_out_index, low32);
    }
#endif // platform
}