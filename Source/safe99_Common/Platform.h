// 작성자: bumpsgoodman
// 작성일: 2024-08-10

#ifndef SAFE99_PLATFORM_H
#define SAFE99_PLATFORM_H

#if defined(_WIN32)
    #define PLATFORM_WIN_X86
#endif // _WIN32

#if defined(_WIN64)
    #define PLATFORM_WIN_X64
#endif // _WIN64

#if defined(PLATFORM_WIN_X86) || defined(PLATFORM_WIN_X64)
    #define UW_PLATFORM_WIN
#endif // PLATFORM_WIN_X86 || PLATFORM_WIN_X64

#if defined(PLATFORM_WIN_X86)
    #define PLATFORM_X86
#endif // PLATFORM_WIN_X86

#if defined(PLATFORM_WIN_X64)
    #define PLATFORM_X64
#endif // PLATFORM_WIN_X64

#endif // SAFE99_PLATFORM_H