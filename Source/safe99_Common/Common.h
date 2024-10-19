// 작성자: bumpsgoodman
// 작성일: 2024-08-10

#ifndef SAFE99_COMMON_H
#define SAFE99_COMMON_H

#include "Assert.h"
#include "ErrorCode.h"
#include "Platform.h"
#include "PrimitiveType.h"
#include "SafeDelete.h"

#define SAFE99_FILE_PATH_LEN 260
#define SAFE99_FILE_NAME_LEN 64

// dll export
#ifdef SAFE99_DLL
    #define SAFE99_GLOBAL_FUNC __declspec(dllexport)
#else
    #define SAFE99_GLOBAL_FUNC __declspec(dllimport)
#endif // SAFE99_DLL

typedef void(__stdcall* CreateDllInstanceFunc)(void** ppOutInstance);

#define SAFE99_INTERFACE struct

// Alignment
#define DEFAULT_ALIGN   16
#define ALIGN8          _declspec(align(8))
#define ALIGN16         _declspec(align(16))
#define ALIGN32         _declspec(align(32))

#endif // SAFE99_COMMON_H