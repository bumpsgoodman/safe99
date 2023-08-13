#include <stdio.h>

#include "ecs.h"

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
        const archetype_t* p_archetype = &p_view->p_archetypes[i];

        position_t* p_pos = (position_t*)ecs_get_instances_or_null(p_view, i, g_pos);
        velocity_t* p_vel = (velocity_t*)ecs_get_instances_or_null(p_view, i, g_vel);

        for (size_t j = 0; j < p_archetype->num_instances; ++j)
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
    ecs_world_t world;
    ecs_init(&world, 10, 2, 2);

    g_pos = ECS_REGISTER_COMPONENT(&world, position_t);
    g_vel = ECS_REGISTER_COMPONENT(&world, velocity_t);

    const ecs_id_t update_pos_system = ECS_REGISTER_SYSTEM(&world, update_pos, 2, g_pos, g_vel);

    for (size_t i = 0; i < 10; ++i)
    {
        const ecs_id_t entity = ecs_create_entity(&world);
        ecs_set_component(&world, entity, g_pos, &(position_t){ (float)i, (float)i + 1.0f });
        ecs_set_component(&world, entity, g_vel, &(velocity_t){ 1.1f, 2.2f });
    }

    ecs_update_system(&world, update_pos_system);

    return 0;
}