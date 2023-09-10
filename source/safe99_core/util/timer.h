#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
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