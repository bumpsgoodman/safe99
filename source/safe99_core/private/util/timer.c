#include "precompiled.h"

bool init_timer(timer_t* p_timer, const float interval)
{
    ASSERT(p_timer != NULL, "timer == NULL");
    ASSERT(interval >= 0.0f, "interval < 0");

    QueryPerformanceFrequency(&p_timer->frequency);
    QueryPerformanceCounter(&p_timer->prev_counter);

    p_timer->interval = interval;
    p_timer->elapsed_tick = 0.0f;

    return true;
}

void update_timer(timer_t* p_timer)
{
    ASSERT(p_timer != NULL, "timer == NULL");

    static LARGE_INTEGER cur_counter;

    QueryPerformanceCounter(&cur_counter);
    const double d_elapsed_tick = ((double)cur_counter.QuadPart - (double)p_timer->prev_counter.QuadPart) / (double)p_timer->frequency.QuadPart * 1000.0;
    p_timer->elapsed_tick = (float)d_elapsed_tick;

    if (is_on_tick_timer(p_timer))
    {
        p_timer->prev_counter = cur_counter;
    }
}

void reset_timer(timer_t* p_timer)
{
    ASSERT(p_timer != NULL, "timer == NULL");
    QueryPerformanceCounter(&p_timer->prev_counter);
    p_timer->elapsed_tick = 0.0f;
}

bool is_on_tick_timer(const timer_t* p_timer)
{
    ASSERT(p_timer != NULL, "timer == NULL");
    return p_timer->elapsed_tick >= p_timer->interval;
}