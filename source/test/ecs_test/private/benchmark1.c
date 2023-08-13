#include <Windows.h>
#include <stdio.h>

#include "ecs.h"
#include "util/assert.h"

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
    ecs_world_t world;
    LARGE_INTEGER frequency;
    LARGE_INTEGER prev_counter;
    LARGE_INTEGER cur_counter;

    const size_t num_max_entities = 1000000;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&prev_counter);

    printf("%llu entities, %llu components에 대한 테스트\n\n", num_max_entities, 2ull);

    printf("초기화 시간 측정\n");
    QueryPerformanceCounter(&prev_counter);

    ecs_init(&world, num_max_entities, 2, 2);

    QueryPerformanceCounter(&cur_counter);
    double d_elapsed_tick = ((double)cur_counter.QuadPart - (double)prev_counter.QuadPart) / (double)frequency.QuadPart * 1000.0;
    float elapsed_tick = (float)d_elapsed_tick;
    printf("초기화 시간: %.3fms\n\n", elapsed_tick);

    g_pos = ECS_REGISTER_COMPONENT(&world, position_t);
    g_vel = ECS_REGISTER_COMPONENT(&world, velocity_t);

    g_update_pos_sys = ECS_REGISTER_SYSTEM(&world, update_pos, 2, g_pos, g_vel);

    printf("entity 생성 시간 측정\n");
    QueryPerformanceCounter(&prev_counter);
    for (ecs_id_t i = 0; i < num_max_entities; ++i)
    {
        ecs_id_t e0 = ecs_create_entity(&world);
        ecs_add_component(&world, e0, 2, g_pos, g_vel);
        //ecs_set_component(&world, e0, g_pos, &(position_t){ 1.0f, 1.0f });
        //ecs_set_component(&world, e0, g_vel, &(velocity_t){ 2.0f, 2.0f });
    }
    QueryPerformanceCounter(&cur_counter);
    d_elapsed_tick = ((double)cur_counter.QuadPart - (double)prev_counter.QuadPart) / (double)frequency.QuadPart * 1000.0;
    elapsed_tick = (float)d_elapsed_tick;
    printf("entity 생성 시간: %.4fms\n\n", elapsed_tick);

    printf("update 시간 측정\n");
    QueryPerformanceCounter(&prev_counter);

    ecs_update_system(&world, g_update_pos_sys);

    QueryPerformanceCounter(&cur_counter);
    d_elapsed_tick = ((double)cur_counter.QuadPart - (double)prev_counter.QuadPart) / (double)frequency.QuadPart * 1000.0;
    elapsed_tick = (float)d_elapsed_tick;
    printf("update 시간: %.4fms\n\n", elapsed_tick);

    printf("entity 제거 시간 측정\n");
    QueryPerformanceCounter(&prev_counter);
    for (ecs_id_t i = 0; i < num_max_entities; ++i)
    {
        ecs_destroy_entity(&world, i | ECS_FLAG_ENTITY);
    }
    QueryPerformanceCounter(&cur_counter);
    d_elapsed_tick = ((double)cur_counter.QuadPart - (double)prev_counter.QuadPart) / (double)frequency.QuadPart * 1000.0;
    elapsed_tick = (float)d_elapsed_tick;
    printf("entity 제거 시간: %.4fms\n\n", elapsed_tick);

    ecs_release(&world);
}

void update_pos(const ecs_view_t* p_view)
{
    for (size_t i = 0; i < p_view->num_archetypes; ++i)
    {
        archetype_t* p_archetype = &p_view->p_archetypes[i];

        position_t* p_pos = (position_t*)ecs_get_instances_or_null(p_view, i, g_pos);
        velocity_t* p_vel = (velocity_t*)ecs_get_instances_or_null(p_view, i, g_vel);

        for (size_t j = 0; j < p_archetype->num_instances; ++j)
        {
            p_pos[j].x += p_vel[j].x;
            p_pos[j].y += p_vel[j].y;

            //printf("pos: [%.3f, %.3f]\n", p_pos[j].x, p_pos[j].y);
            //printf("vel: [%.3f, %.3f]\n\n", p_vel[j].x, p_vel[j].y);
        }
    }
}