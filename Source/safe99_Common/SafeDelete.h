// 작성자: bumpsgoodman
// 작성일: 2024-08-10

#ifndef SAFE99_SAFE_DELETE_H
#define SAFE99_SAFE_DELETE_H

#include <stdlib.h>
#include <Windows.h>

#ifdef __cplusplus
    #define SAFE_RELEASE(p)         { if ((p)) { (p)->Release(); (p) = NULL; } }
#else
    #define SAFE_RELEASE(p)         { if ((p)) { (p)->Release((p)); (p) = NULL; } }
#endif // __cplusplus

#define SAFE_FREE(p)                { if ((p)) free((p)); (p) = NULL; }
#define SAFE_VIRTUAL_FREE(p)        { VirtualFree((p), 0, MEM_RELEASE); (p) = NULL; }
#define SAFE_FREE_LIBRARY(handle)   { if ((handle)) { FreeLibrary(handle); (handle) = NULL; } }

#endif // SAFE99_SAFE_DELETE_H