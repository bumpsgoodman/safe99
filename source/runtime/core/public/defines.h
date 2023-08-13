#ifndef DEFINES_H
#define DEFINES_H

// dll exports
#ifdef SAFE99_DLL_EXPORTS
#define SAFE99_API __declspec(dllexport)
#else
#define SAFE99_API __declspec(dllimport)
#endif // SAFE99_DLL_EXPORTS

// extern "C"
#ifdef __cplusplus
#define START_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define START_EXTERN_C
#define END_EXTERN_C
#endif // __cplusplus

// inline
#ifdef _MSC_VER
#define INLINE inline
#define FORCEINLINE __forceinline
#endif // _MSC_VER

#define TO_STR(s) #s

#endif // DEFINES_H