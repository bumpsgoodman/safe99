//***************************************************************************
// 
// 파일: stop_watch.h
// 
// 설명: 스톱워치
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/10/01
// 
//***************************************************************************

#ifndef STOP_WATCH_H
#define STOP_WATCH_H

#include <Windows.h>

#include "safe99_common/defines.h"

typedef struct stop_watch
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER prev_counter;

    float elapsed_tick;
} stop_watch_t;

START_EXTERN_C

SAFE99_API bool stop_watch_init(stop_watch_t* p_stop_watch);

SAFE99_API void stop_watch_start(stop_watch_t* p_stop_watch);

SAFE99_API void stop_watch_end(stop_watch_t* p_stop_watch);

SAFE99_API float stop_watch_get_elapsed_tick(const stop_watch_t* p_stop_watch);

END_EXTERN_C

#endif // STOP_WATCH_H