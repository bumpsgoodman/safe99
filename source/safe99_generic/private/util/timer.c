//***************************************************************************
// 
// 파일: timer.c
// 
// 설명: 타이머
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/08/14
// 
//***************************************************************************

#include "precompiled.h"

bool __stdcall timer_init(timer_t* p_timer)
{
    ASSERT(p_timer != NULL, "p_timer == NULL");

    QueryPerformanceFrequency((LARGE_INTEGER*)&p_timer->frequency);

    return true;
}

void __stdcall timer_start(timer_t* p_timer)
{
    ASSERT(p_timer != NULL, "p_timer == NULL");
    QueryPerformanceCounter((LARGE_INTEGER*)&p_timer->prev_counter);
}

float __stdcall timer_get_time(const timer_t* p_timer)
{
    ASSERT(p_timer != NULL, "p_timer == NULL");

    uint64_t cur_counter;
    QueryPerformanceCounter((LARGE_INTEGER*)&cur_counter);
    return (float)(((double)cur_counter - (double)p_timer->prev_counter) / (double)p_timer->frequency);
}