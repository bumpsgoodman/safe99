#include <stdio.h>
#include <Windows.h>
#include <crtdbg.h>

#include "safe99_common/assert.h"
#include "safe99_ecs/i_ecs.h"

typedef struct
{
    float x;
    float y;
} position_t, velocity_t;

ecs_id_t g_pos;
ecs_id_t g_vel;

ecs_id_t g_update_pos_sys;

void update_pos(const ecs_view_t* p_view);

int main(void)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    HINSTANCE ecs_dll = LoadLibrary(L"dll\\safe99_ecs_x64d.dll");
    if (ecs_dll == NULL)
    {
        return 0;
    }

    create_instance_func f = (create_instance_func)GetProcAddress(ecs_dll, "create_instance");
    i_ecs_t* ecs;
    f((void**)&ecs);

    LARGE_INTEGER frequency;
    LARGE_INTEGER prev_counter;
    LARGE_INTEGER cur_counter;

    const size_t num_max_entities = 1000000;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&prev_counter);

    printf("%llu entities, %llu components에 대한 테스트\n\n", num_max_entities, 2ull);

    printf("초기화 시간 측정\n");
    QueryPerformanceCounter(&prev_counter);

    ecs->vtbl->initialize(ecs, num_max_entities, 2, 2);

    QueryPerformanceCounter(&cur_counter);
    double d_elapsed_tick = ((double)cur_counter.QuadPart - (double)prev_counter.QuadPart) / (double)frequency.QuadPart * 1000.0;
    float elapsed_tick = (float)d_elapsed_tick;
    printf("초기화 시간: %.3fms\n\n", elapsed_tick);

    g_pos = ECS_REGISTER_COMPONENT(ecs, position_t);
    g_vel = ECS_REGISTER_COMPONENT(ecs, velocity_t);

    g_update_pos_sys = ECS_REGISTER_SYSTEM(ecs, update_pos, 2, g_pos, g_vel);

    printf("entity 생성 시간 측정\n");
    QueryPerformanceCounter(&prev_counter);
    for (ecs_id_t i = 0; i < num_max_entities; ++i)
    {
        ecs_id_t e0 = ecs->vtbl->create_entity(ecs);

        ecs->vtbl->add_component(ecs, e0, 2, g_pos, g_vel);

        ecs->vtbl->set_component(ecs, e0, g_pos, &(position_t){ 1.0f, 1.0f });
        ecs->vtbl->set_component(ecs, e0, g_vel, &(velocity_t){ 2.0f, 2.0f });
    }
    QueryPerformanceCounter(&cur_counter);
    d_elapsed_tick = ((double)cur_counter.QuadPart - (double)prev_counter.QuadPart) / (double)frequency.QuadPart * 1000.0;
    elapsed_tick = (float)d_elapsed_tick;
    printf("entity 생성 시간: %.4fms\n\n", elapsed_tick);

    printf("update 시간 측정\n");
    QueryPerformanceCounter(&prev_counter);

    ecs->vtbl->update_system(ecs, g_update_pos_sys);

    QueryPerformanceCounter(&cur_counter);
    d_elapsed_tick = ((double)cur_counter.QuadPart - (double)prev_counter.QuadPart) / (double)frequency.QuadPart * 1000.0;
    elapsed_tick = (float)d_elapsed_tick;
    printf("update 시간: %.4fms\n\n", elapsed_tick);

    printf("entity 제거 시간 측정\n");
    QueryPerformanceCounter(&prev_counter);
    for (ecs_id_t i = 0; i < num_max_entities; ++i)
    {
        ecs->vtbl->destroy_entity(ecs, i | ECS_FLAG_ENTITY);
    }
    QueryPerformanceCounter(&cur_counter);
    d_elapsed_tick = ((double)cur_counter.QuadPart - (double)prev_counter.QuadPart) / (double)frequency.QuadPart * 1000.0;
    elapsed_tick = (float)d_elapsed_tick;
    printf("entity 제거 시간: %.4fms\n\n", elapsed_tick);

    ecs->vtbl->release(ecs);

    _CrtDumpMemoryLeaks();

    return 0;
}

void update_pos(const ecs_view_t* p_view)
{
    for (size_t i = 0; i < p_view->num_archetypes; ++i)
    {
        position_t* p_pos = (position_t*)p_view->p_this->vtbl->get_instances_or_null(p_view, i, g_pos);
        velocity_t* p_vel = (velocity_t*)p_view->p_this->vtbl->get_instances_or_null(p_view, i, g_vel);

        const size_t num_instances = p_view->p_this->vtbl->get_num_instances(p_view, i);
        for (size_t j = 0; j < num_instances; ++j)
        {
            p_pos[j].x += p_vel[j].x;
            p_pos[j].y += p_vel[j].y;
        }
    }
}