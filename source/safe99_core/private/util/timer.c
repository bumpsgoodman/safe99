#include "precompiled.h"

bool timer_init(timer_t* p_timer, const float interval)
{
    ASSERT(p_timer != NULL, "timer == NULL");
    ASSERT(interval >= 0.0f, "interval < 0");

    QueryPerformanceFrequency(&p_timer->frequency);
    QueryPerformanceCounter(&p_timer->prev_counter);

    p_timer->interval = interval;
    p_timer->elapsed_tick = 0.0f;

    return true;
}

void timer_update(timer_t* p_timer)
{
    ASSERT(p_timer != NULL, "timer == NULL");

    static LARGE_INTEGER cur_counter;

    QueryPerformanceCounter(&cur_counter);
    const double d_elapsed_tick = ((double)cur_counter.QuadPart - (double)p_timer->prev_counter.QuadPart) / (double)p_timer->frequency.QuadPart * 1000.0;
    p_timer->elapsed_tick = (float)d_elapsed_tick;

    if (timer_is_on_tick(p_timer))
    {
        p_timer->prev_counter = cur_counter;
    }
}

void timer_reset(timer_t* p_timer)
{
    ASSERT(p_timer != NULL, "timer == NULL");
    QueryPerformanceCounter(&p_timer->prev_counter);
    p_timer->elapsed_tick = 0.0f;
}

bool timer_is_on_tick(const timer_t* p_timer)
{
    ASSERT(p_timer != NULL, "timer == NULL");
    return p_timer->elapsed_tick >= p_timer->interval;
}