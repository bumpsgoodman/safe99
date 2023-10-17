//***************************************************************************
// 
// 파일: hash_function.c
// 
// 설명: 해시 함수
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/07/18
// 
//***************************************************************************

#include "precompiled.h"

uint32_t hash32_fnv1a(const char* bytes, const size_t len)
{
    static const uint32_t FNV_PRIME_32 = 16777619U;
    static const uint32_t FNV_OFFSET_32 = 2166136261U;

    uint32_t hash = FNV_OFFSET_32;
    for (uint32_t i = 0; i < len; ++i)
    {
        hash = hash * FNV_PRIME_32;
        hash = hash + bytes[i];
    }

    return hash;
}

uint64_t hash64_fnv1a(const char* bytes, const size_t len)
{
    ASSERT(bytes != NULL, "bytes == NULL");

    static const uint64_t FNV_PRIME = 1099511628211U;
    static const uint64_t FNV_OFFSET_BASIS = 14695981039346656037U;

    uint64_t hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < len; ++i)
    {
        hash = hash ^ bytes[i];
        hash = hash * FNV_PRIME;
    }

    return hash;
}