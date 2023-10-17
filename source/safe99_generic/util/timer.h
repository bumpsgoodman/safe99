//***************************************************************************
// 
// 파일: timer.h
// 
// 설명: 타이머
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/08/14
// 
//***************************************************************************

#ifndef TIMER_H
#define TIMER_H

#include <Windows.h>

#include "safe99_common/defines.h"

typedef struct timer
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER prev_counter;

    float interval;
    float elapsed_tick;
} timer_t;

START_EXTERN_C

SAFE99_API bool timer_init(timer_t* p_timer, const float interval);

SAFE99_API void timer_update(timer_t* p_timer);

SAFE99_API void timer_reset(timer_t* p_timer);

SAFE99_API bool timer_is_on_tick(const timer_t* p_timer);

END_EXTERN_C

#endif // TIMER_H