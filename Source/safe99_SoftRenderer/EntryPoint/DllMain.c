// 작성자: bumpsgoodman
// 작성일: 2024-08-24

#include "Precompiled.h"

BOOL APIENTRY DllMain(HINSTANCE hModule , DWORD fdwReason , LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    default:
        break;
    }

    return TRUE;
}