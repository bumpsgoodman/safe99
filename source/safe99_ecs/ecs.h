#ifndef ECS_H
#define ECS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "safe99_common/defines.h"
#include "safe99_core/generic/chunked_memory_pool.h"
#include "safe99_core/generic/dynamic_vector.h"
#include "safe99_core/generic/map.h"

// id 구성, 총 64비트
// -----------------------------------------------------------------
// |   8   |       16        |          16          |      24      |
// ----------------------------------------------------------------|
// |  flag |      free       |   entity_generation  |  entity_id   |
// -----------------------------------------------------------------

// 상수 정의
// -----------------------------------------------------------------

#define ECS_NUM_MAX_ENTITIES 16777216ull
#define ECS_MAX_ENTITY_GEN 65536ull

#define ECS_FLAG_ENTITY 0x100000000000000ull
#define ECS_FLAG_COMPONENT 0x200000000000000ull
#define ECS_FLAG_SYSTEM 0x400000000000000ull
#define ECS_FLAG_RESERVED1 0x1000000000000000ull
#define ECS_FLAG_RESERVED2 0x2000000000000000ull
#define ECS_FLAG_RESERVED3 0x4000000000000000ull
#define ECS_FLAG_DISABLE 0x8000000000000000ull

// -----------------------------------------------------------------

// 타입 정의
// -----------------------------------------------------------------

typedef uint64_t ecs_id_t;
typedef uint64_t ecs_hash_t;

// -----------------------------------------------------------------

// struct 전방 선언
// -----------------------------------------------------------------

typedef struct ecs_world ecs_world_t;

typedef struct entity_queue entity_queue_t;
typedef struct entity_field entity_field_t;

typedef struct ecs_mask ecs_mask_t;
typedef struct archetype archetype_t;
typedef struct archetype_map archetype_map_t;

typedef struct system system_t;
typedef struct ecs_view ecs_view_t;

typedef void(*ecs_system_func)(const ecs_view_t*);

// -----------------------------------------------------------------

// struct 정의
// -----------------------------------------------------------------

typedef struct entity_queue
{
    ecs_id_t* pa_entities;
    size_t front;
    size_t rear;
} entity_queue_t;

typedef struct entity_field
{
    ecs_id_t id;
    archetype_t* p_archetype;
    size_t col;
} entity_field_t;

typedef struct ecs_mask
{
    uint64_t* p_masks;
    size_t num_components;
    ecs_hash_t hash;
} ecs_mask_t;

typedef struct archetype
{
    ecs_mask_t mask;
    map_t component_map;                        // { component : index }
    size_t num_instances;

    dynamic_vector_t* pa_instances_array;       // { char* }
    dynamic_vector_t entities;                  // { ecs_id_t }
} archetype_t;

typedef struct ecs_view
{
    ecs_world_t* p_world;
    archetype_t* p_archetypes;
    size_t num_archetypes;
} ecs_view_t;

typedef struct system
{
    ecs_mask_t mask;
    dynamic_vector_t archetypes;                // <archetype*>
    ecs_system_func p_func;
} system_t;

typedef struct ecs_world
{
    // 공용
    size_t num_max_entities;
    size_t num_max_components;
    size_t num_max_systems;

    chunked_memory_pool_t mask_pool;

    // entity
    entity_field_t* pa_entity_fields;
    ecs_id_t* pa_no_archetype_entities;
    entity_queue_t destroyed_entities;
    size_t num_entities;
    size_t num_no_archetype_entities;
    size_t entity_count;

    // component
    map_t registered_components;                 // { component name hash : component }
    size_t* pa_component_size;                   // component id가 인덱스

    // archetype
    map_t archetype_map;                         // { component hash : archetype }
    size_t num_archetypes;

    // system
    map_t registered_systems;
    system_t* pa_systems;
} ecs_world_t;

// -----------------------------------------------------------------

START_EXTERN_C

SAFE99_API bool ecs_init(ecs_world_t* p_world,
                         const size_t num_max_entities,
                         const size_t num_max_components,
                         const size_t num_max_systems);

SAFE99_API void ecs_release(ecs_world_t* p_world);


// component 등록 실패 시 0 반환
SAFE99_API ecs_id_t ecs_register_component(ecs_world_t* p_world, const char* p_component_name, const size_t component_size);

// system 등록 실패 시 0 반환
SAFE99_API ecs_id_t ecs_register_system(ecs_world_t* p_world, const char* p_system_name, ecs_system_func p_func, const size_t num_components, ...);

// 등록되지 않은 component는 0 반환
SAFE99_API ecs_id_t ecs_get_component_id(ecs_world_t* p_world, const char* p_component_name);

// 등록되지 않은 system은 0 반환
SAFE99_API ecs_id_t ecs_get_system_id(ecs_world_t* p_world, const char* p_system_name);

// entity 생성 실패 시 0 반환
SAFE99_API ecs_id_t ecs_create_entity(ecs_world_t* p_world);

SAFE99_API bool ecs_destroy_entity(ecs_world_t* p_world, const ecs_id_t entity);

SAFE99_API bool ecs_is_alive_entity(ecs_world_t* p_world, const ecs_id_t entity);

SAFE99_API bool ecs_has_component(ecs_world_t* p_world, const ecs_id_t entity, const size_t num_components, ...);

// 추가할 component가 한 개도 없다면 false 반환
SAFE99_API bool ecs_add_component(ecs_world_t* p_world, const ecs_id_t entity, const size_t num_components, ...);

// entity에 component가 없다면 추가 후 초기화
SAFE99_API bool ecs_set_component(ecs_world_t* p_world, const ecs_id_t entity, const ecs_id_t component, void* p_value);

// 제거할 component가 한 개도 없다면 false 반환
SAFE99_API bool ecs_remove_component(ecs_world_t* p_world, const ecs_id_t entity, const size_t num_components, ...);

SAFE99_API bool ecs_update_system(ecs_world_t* p_world, const ecs_id_t system);

SAFE99_API void* ecs_get_instances_or_null(const ecs_view_t* p_view, const size_t archetype_index, const ecs_id_t component);

SAFE99_API void* ecs_get_entities_or_null(const ecs_view_t* p_view, const size_t archetype_index);

END_EXTERN_C

#define ECS_REGISTER_COMPONENT(p_world, component) \
    ecs_register_component((p_world), TO_STR(component), sizeof(component))

#define ECS_REGISTER_SYSTEM(p_world, system, num_components, ...) \
    ecs_register_system((p_world), TO_STR(system), system, (num_components), __VA_ARGS__)

#define ECS_GET_COMPONENT_ID(p_world, component) \
    ecs_get_component_id((p_world), TO_STR(component))

#define ECS_GET_SYSTEM_ID(p_world, system) \
    ecs_get_system_id((p_world), TO_STR(system))

#endif // ECS_H