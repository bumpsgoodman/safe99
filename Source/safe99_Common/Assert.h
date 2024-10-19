// 작성자: bumpsgoodman
// 작성일: 2024-08-10

#ifndef SAFE99_ASSERT_H
#define SAFE99_ASSERT_H

#include <intrin.h>

#define CRASH(msg)   (__debugbreak())

#if defined(NDEBUG)
    #define ASSERT(cond, msg) ((void)0)
#else
    #define ASSERT(cond, msg) { if (!(cond)) { __debugbreak(); } }
#endif // NDEBUG

#endif // SAFE99_ASSERT_H