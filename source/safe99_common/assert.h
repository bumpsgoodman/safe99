#ifndef ASSERT_H
#define ASSERT_H

#if defined(NDEBUG)
    #define ASSERT(cond, msg) ((void)0)
#else
    #ifdef _MSC_VER
        #include <intrin.h>
        #define ASSERT(cond, msg) { if (!(cond)) { __debugbreak(); } }
    #endif // _MSC_VER
#endif // NDBUG

#endif // ASSERT_H