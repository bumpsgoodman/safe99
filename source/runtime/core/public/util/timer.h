#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <Windows.h>

#include "defines.h"

typedef struct timer
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER prev_counter;

    float interval;
    float elapsed_tick;
} timer_t;

START_EXTERN_C

SAFE99_API bool init_timer(timer_t* p_timer, const float interval);

SAFE99_API void update_timer(timer_t* p_timer);

SAFE99_API void reset_timer(timer_t* p_timer);

SAFE99_API bool is_on_tick_timer(const timer_t* p_timer);

END_EXTERN_C

#endif // TIMER_H