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

#include "safe99_common/defines.h"

START_EXTERN_C

SAFE99_API bool __stdcall timer_init(timer_t* p_timer);
SAFE99_API float __stdcall timer_get_time(const timer_t* p_timer);

END_EXTERN_C

#endif // TIMER_H