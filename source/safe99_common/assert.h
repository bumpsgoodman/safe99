#ifndef ASSERT_H
#define ASSERT_H

#include <assert.h>

#if defined(NDEBUG)
    #define ASSERT(cond, msg) ((void)0)
#else
    #include <intrin.h>
    #define ASSERT(cond, msg) { if (!(cond)) { __debugbreak(); } }
#endif // NDBUG

#endif // ASSERT_H