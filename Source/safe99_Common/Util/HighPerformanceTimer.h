// 작성자: bumpsgoodman
// 작성일: 2024-08-27

#ifndef SAFE99_HIGH_PERFORMANCE_TIMER_H
#define SAFE99_HIGH_PERFORMANCE_TIMER_H

typedef struct HIGH_PERFORMANCE_TIMER
{
    uint64_t Frequency;
    uint64_t PrevCounter;
    float InverseFrequency;
    float DeltaTime;
} HIGH_PERFORMANCE_TIMER;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

bool HighPerformanceTimerInit(HIGH_PERFORMANCE_TIMER* pTimer);
void HighPerformanceTimerUpdate(HIGH_PERFORMANCE_TIMER* pTimer, const float tick);
float HighPerformanceTimerGetDeltaTime(const HIGH_PERFORMANCE_TIMER* pTimer);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SAFE99_HIGH_PERFORMANCE_TIMER_H