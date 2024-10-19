// 작성자: bumpsgoodman
// 작성일: 2024-09-18

#ifndef SAFE99_PRECOMPILED_H
#define SAFE99_PRECOMPILED_H

#define _CRT_SECURE_NO_WARNINGS
#define SAFE99_DLL

#include "safe99_Common/Platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

// safe99 library
#if defined(PLATFORM_X64)
    #if defined(_DEBUG)
        #pragma comment(lib, "safe99_Generic_x64d.lib")
    #else
        #pragma comment(lib, "safe99_Generic_x64.lib")
    #endif
#elif defined(PLATFORM_X86)
    #if defined(_DEBUG)
        #pragma comment(lib, "safe99_Generic_x86d.lib")
    #else
        #pragma comment(lib, "safe99_Generic_x86.lib")
    #endif
#endif

#endif // SAFE99_PRECOMPILED_H