#include <stdio.h>
#include <Windows.h>
#include <crtdbg.h>

#include "safe99_ecs/i_ecs.h"

typedef struct
{
    float x;
    float y;
} position_t, velocity_t;

ecs_id_t g_pos;
ecs_id_t g_vel;

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

            printf("pos: [%.3f, %.3f]\n", p_pos[j].x, p_pos[j].y);
            printf("vel: [%.3f, %.3f]\n\n", p_vel[j].x, p_vel[j].y);
        }
    }
}

int main(void)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    //_CrtSetBreakAlloc(67);

    HINSTANCE ecs_dll = LoadLibrary(L"dll\\safe99_ecs_x64d.dll");
    if (ecs_dll == NULL)
    {
        return 0;
    }

    create_instance_func f = (create_instance_func)GetProcAddress(ecs_dll, "create_instance");
    i_ecs_t* ecs;
    f((void**)&ecs);

    ecs->vtbl->initialize(ecs, 2, 2, 2);

#if 1
    g_pos = ECS_REGISTER_COMPONENT(ecs, position_t);
    g_vel = ECS_REGISTER_COMPONENT(ecs, velocity_t);

    const ecs_id_t update_pos_system = ECS_REGISTER_SYSTEM(ecs, update_pos, 2, g_pos, g_vel);

    for (size_t i = 0; i < 2; ++i)
    {
        const ecs_id_t entity = ecs->vtbl->create_entity(ecs);
        ecs->vtbl->set_component(ecs, entity, g_pos, &(position_t){ (float)i, (float)i + 1.0f });
        ecs->vtbl->set_component(ecs, entity, g_vel, &(velocity_t){ 1.1f, 2.2f });
    }
    ecs->vtbl->update_system(ecs, update_pos_system);
#endif
    
    ecs->vtbl->release(ecs);

    _CrtDumpMemoryLeaks();

    return 0;
}