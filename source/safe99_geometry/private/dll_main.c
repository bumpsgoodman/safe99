//***************************************************************************
// 
// 파일: dll_main.c
// 
// 설명: dll main
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/07/11
// 
//***************************************************************************

#include "precompiled.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}