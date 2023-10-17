//***************************************************************************
// 
// 파일: stop_watch.c
// 
// 설명: 스톱워치
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/10/01
// 
//***************************************************************************

#include "precompiled.h"

bool stop_watch_init(stop_watch_t* p_stop_watch)
{
    ASSERT(p_stop_watch != NULL, "p_stop_watch == NULL");

    QueryPerformanceFrequency(&p_stop_watch->frequency);

    p_stop_watch->elapsed_tick = 0.0f;

    return true;
}

void stop_watch_start(stop_watch_t* p_stop_watch)
{
    ASSERT(p_stop_watch != NULL, "p_stop_watch == NULL");
    QueryPerformanceCounter(&p_stop_watch->prev_counter);
}

void stop_watch_end(stop_watch_t* p_stop_watch)
{
    ASSERT(p_stop_watch != NULL, "p_stop_watch == NULL");

    LARGE_INTEGER cur_counter;
    QueryPerformanceCounter(&cur_counter);

    const double d_elapsed_tick = ((double)cur_counter.QuadPart - (double)p_stop_watch->prev_counter.QuadPart) / (double)p_stop_watch->frequency.QuadPart * 1000.0;
    p_stop_watch->elapsed_tick = (float)d_elapsed_tick;
}

float stop_watch_get_elapsed_tick(const stop_watch_t* p_stop_watch)
{
    ASSERT(p_stop_watch != NULL, "p_stop_watch == NULL");
    return p_stop_watch->elapsed_tick;
}