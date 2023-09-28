//***************************************************************************
// 
// 파일: defines.h
// 
// 설명: safe99 전용 매크로 정의
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/09/04
// 
//***************************************************************************

#ifndef DEFINES_H
#define DEFINES_H

// extern "C"
#ifdef __cplusplus
#define START_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define START_EXTERN_C
#define END_EXTERN_C
#endif // __cplusplus

// calling convention
#define STDCALL __stdcall
#define VECTORCALL __vectorcall

// SIMD
#if (defined(__AVX2__))
#define SUPPORT_AVX2
#endif // AVX2
#if (defined(__AVX__))
#define SUPPORT_AVX
#endif // AVX
#if (defined(_M_AMD64) || defined(_M_X64) || defined(__x86_64__) || defined(_M_IX86) || defined(__i386__))
#define SUPPORT_SSE
#endif // SSE

// inline
#define INLINE inline
#define FORCEINLINE __forceinline

// alignment
#ifdef _MSC_VER
#define ALIGN8 _declspec(align(8))
#define ALIGN16 _declspec(align(16))
#define ALIGN32 _declspec(align(32))
#endif // _MSC_VER

// dll exports
#ifdef SAFE99_DLL_EXPORTS
#define SAFE99_API __declspec(dllexport)
#else
#define SAFE99_API __declspec(dllimport)
#endif // SAFE99_DLL_EXPORTS

#define TO_STR(s) #s

#endif // DEFINES_H