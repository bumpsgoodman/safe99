//***************************************************************************
// 
// 파일: hash_function.h
// 
// 설명: 해시 함수
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/07/18
// 
//***************************************************************************

#ifndef HASH_FUNCTION_H
#define HASH_FUNCTION_H

#include "safe99_common/defines.h"

START_EXTERN_C

SAFE99_API uint32_t __stdcall hash32_fnv1a(const char* bytes, const size_t size);
SAFE99_API uint64_t __stdcall hash64_fnv1a(const char* bytes, const size_t size);

END_EXTERN_C

#endif // HASH_FUNCTION_H