// 작성자: bumpsgoodman
// 작성일: 2024-08-27

#include "Precompiled.h"

#include "../Common.h"
#include "HighPerformanceTimer.h"

#include <Windows.h>

bool HighPerformanceTimerInit(HIGH_PERFORMANCE_TIMER* pTimer)
{
    ASSERT(pTimer != NULL, "pTimer is NULL");
    
    QueryPerformanceFrequency((LARGE_INTEGER*)&pTimer->Frequency);
    QueryPerformanceCounter((LARGE_INTEGER*)&pTimer->PrevCounter);
    pTimer->InverseFrequency = 1.0f / (float)pTimer->Frequency;
    pTimer->DeltaTime = 0.0f;

    return true;
}

void HighPerformanceTimerUpdate(HIGH_PERFORMANCE_TIMER* pTimer, const float tick)
{
    ASSERT(pTimer != NULL, "pTimer is NULL");
    
    uint64_t curCounter;
    QueryPerformanceCounter((LARGE_INTEGER*)&curCounter);

    pTimer->DeltaTime = (float)(curCounter - pTimer->PrevCounter) * pTimer->InverseFrequency;
    if (pTimer->DeltaTime >= tick)
    {
        pTimer->PrevCounter = curCounter;
    }
}

float HighPerformanceTimerGetDeltaTime(const HIGH_PERFORMANCE_TIMER* pTimer)
{
    ASSERT(pTimer != NULL, "pTimer is NULL");
    return pTimer->DeltaTime;
}