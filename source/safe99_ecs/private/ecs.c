//***************************************************************************
// 
// 파일: ecs.c
// 
// 설명: safe99 ecs
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/07/05
// 
//***************************************************************************

#include "precompiled.h"

#include "i_ecs.h"
#include "safe99_common/defines.h"

// 상수 정의
// -----------------------------------------------------------------

#define ECS_NUM_MAX_ENTITIES 16777216ull
#define ECS_MAX_ENTITY_GEN 65536ull

#define ENTITY_GEN_FLAG 0x1000000ull
#define MAX_ENTITY_GEN_FLAG 0xffff000000ull

// -----------------------------------------------------------------
// 매크로 함수
// -----------------------------------------------------------------

#define NUM_MASKS(num_max_components) (((num_max_components) >> 3) + 1)
#define PURE_ECS_ID(ecs_id) (0xffffffull & (ecs_id))
#define ENTITY_GEN(entity) (0xffff000000ull & (entity))
#define PURE_ENTITY_GEN(entity) ((0xffff000000ull & (entity)) >> 24)

#define IS_ENTITY(entity) ((ECS_FLAG_ENTITY & entity) != 0)
#define IS_COMPONENT(component) ((ECS_FLAG_COMPONENT & component) != 0)
#define IS_SYSTEM(system) ((ECS_FLAG_SYSTEM & system) != 0)

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

typedef struct system
{
    ecs_mask_t mask;
    dynamic_vector_t archetypes;                // <archetype*>
    ecs_system_func p_func;
} system_t;

typedef struct ecs_world
{
    i_ecs_t base;
    size_t ref_count;

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

// 내부 함수 선언
// -----------------------------------------------------------------

static bool create_archetype(ecs_world_t* p_world, const ecs_mask_t* p_mask, archetype_t** pp_out_archetype);

static FORCEINLINE entity_field_t* get_entity_field_or_null(const ecs_world_t* p_world, const ecs_id_t entity);

static bool move_archetype(ecs_world_t* p_world, const ecs_id_t entity, archetype_t* p_to_archetype);
static bool move_archetype_from_null(ecs_world_t* p_world, const ecs_id_t entity, archetype_t* p_to_archetype);
static bool move_archetype_to_null(ecs_world_t* p_world, const ecs_id_t entity);

void __stdcall create_instance(void** pp_out_instance);

// -----------------------------------------------------------------

size_t __stdcall add_ref(i_ecs_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    ecs_world_t* p_world = (ecs_world_t*)p_this;
    return ++p_world->ref_count;
}

size_t __stdcall release(i_ecs_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    ecs_world_t* p_world = (ecs_world_t*)p_this;

    if (--p_world->ref_count == 0)
    {
        // system 해제
        {
            system_t* p_system = p_world->pa_systems;
            for (size_t i = 0; i < p_world->num_max_systems; ++i)
            {
                dynamic_vector_release(&p_system->archetypes);
                ++p_system;
            }

            SAFE_FREE(p_world->pa_systems);

            map_release(&p_world->registered_systems);
        }

        // archetype 해제
        {
            key_value_t* p_key_values = map_get_key_values_ptr(&p_world->archetype_map);
            for (size_t i = 0; i < p_world->num_archetypes; ++i)
            {
                archetype_t* p_archetype = (archetype_t*)p_key_values[i].p_value;

                // instance 해제
                dynamic_vector_t* p_instances = p_archetype->pa_instances_array;
                for (size_t i = 0; i < p_archetype->mask.num_components; ++i)
                {
                    ASSERT(p_instances != NULL, "p_instances == NULL");
                    dynamic_vector_release(&p_instances[i]);
                }
                SAFE_FREE(p_instances);

                dynamic_vector_release(&p_archetype->entities);
                map_release(&p_archetype->component_map);
            }

            map_release(&p_world->archetype_map);
        }

        // component 해제
        {
            SAFE_FREE(p_world->pa_component_size);
            map_release(&p_world->registered_components);
        }

        // entity 해제
        {
            SAFE_FREE(p_world->destroyed_entities.pa_entities);
            SAFE_FREE(p_world->pa_no_archetype_entities);
            SAFE_FREE(p_world->pa_entity_fields);
        }

        // 공용 자원 해제
        chunked_memory_pool_release(&p_world->mask_pool);

        SAFE_FREE(p_world);

        return 0;
    }

    return p_world->ref_count;
}

size_t __stdcall get_ref_count(const i_ecs_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    ecs_world_t* p_world = (ecs_world_t*)p_this;
    return p_world->ref_count;
}

bool __stdcall initialize(i_ecs_t* p_this, const size_t num_max_entities, const size_t num_max_components, const size_t num_max_systems)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(num_max_entities - 1 < ECS_NUM_MAX_ENTITIES, "Out of range");
    ASSERT(num_max_components - 1 < ECS_NUM_MAX_ENTITIES, "Out of range");
    ASSERT(num_max_systems - 1 < ECS_NUM_MAX_ENTITIES, "Out of range");

    ecs_world_t* p_world = (ecs_world_t*)p_this;

    p_world->num_max_entities = num_max_entities;
    p_world->num_max_components = num_max_components;
    p_world->num_max_systems = num_max_systems;

    // 한 노드 당 archetype 개수는 (log2(c) + 1)^2
    // c는 component 개수
    // c = 1024일 때,
    // 한 노드 당 archetype 개수는 11 * 11 = 121
    uint_t log2c1 = 0;
    log2int64(&log2c1, num_max_components);
    ++log2c1;
    const size_t pow2_log2c1 = (size_t)log2c1 * log2c1;

    if (!chunked_memory_pool_initialize(&p_world->mask_pool, sizeof(uint64_t) * NUM_MASKS(num_max_components), pow2_log2c1 * 2))
    {
        ASSERT(false, "Failed to init component mask pool");
        goto failed_init_component_mask_pool;
    }

    // entity 초기화
    {
        p_world->pa_entity_fields = (entity_field_t*)malloc(sizeof(entity_field_t) * num_max_entities);
        if (p_world->pa_entity_fields == NULL)
        {
            ASSERT(false, "Failed to malloc entity fields");
            goto failed_malloc_entity_fields;
        }
        memset(p_world->pa_entity_fields, 0, sizeof(entity_field_t) * num_max_entities);

        p_world->pa_no_archetype_entities = (ecs_id_t*)malloc(sizeof(ecs_id_t) * num_max_entities);
        if (p_world->pa_no_archetype_entities == NULL)
        {
            ASSERT(false, "Failed to malloc no archetype entities");
            goto failed_malloc_no_archetype_entities;
        }
        memset(p_world->pa_no_archetype_entities, 0, sizeof(ecs_id_t) * num_max_entities);

        p_world->destroyed_entities.pa_entities = (ecs_id_t*)malloc(sizeof(ecs_id_t) * (num_max_entities + 1));
        if (p_world->destroyed_entities.pa_entities == NULL)
        {
            ASSERT(false, "Failed to malloc destroyed entities queue");
            goto failed_malloc_destroyed_entities_queue;
        }
        p_world->destroyed_entities.front = 0;
        p_world->destroyed_entities.rear = 0;

        p_world->num_entities = 0;
        p_world->num_no_archetype_entities = 0;
        p_world->entity_count = 0;

        for (size_t i = 0; i < num_max_entities; ++i)
        {
            (p_world->pa_entity_fields + i)->id = i | ECS_FLAG_ENTITY;
        }
    }

    // component 초기화
    {
        if (!map_initialize(&p_world->registered_components, sizeof(ecs_hash_t), sizeof(ecs_id_t), num_max_components))
        {
            ASSERT(false, "Failed to init registered component map");
            goto failed_init_registered_component_map;
        }

        p_world->pa_component_size = (size_t*)malloc(sizeof(size_t) * num_max_components);
        if (p_world->pa_component_size == NULL)
        {
            ASSERT(false, "Failed to malloc component size");
            goto failed_malloc_component_size;
        }
        memset(p_world->pa_component_size, 0, sizeof(size_t) * num_max_components);
    }

    // archetype 초기화
    {
        if (!map_initialize(&p_world->archetype_map, sizeof(ecs_hash_t), sizeof(archetype_t), pow2_log2c1))
        {
            ASSERT(false, "Failed to init archetype map");
            goto failed_init_archetype_map;
        }

        p_world->num_archetypes = 0;
    }

    // system 초기화
    {
        if (!map_initialize(&p_world->registered_systems, sizeof(ecs_hash_t), sizeof(ecs_id_t), num_max_systems))
        {
            ASSERT(false, "Failed to init registered systems map");
            goto failed_init_registered_system_map;
        }

        p_world->pa_systems = (system_t*)malloc(sizeof(system_t) * num_max_systems);
        if (p_world->pa_systems == NULL)
        {
            ASSERT(false, "Failed to malloc systems");
            goto failed_malloc_systems;
        }
        memset(p_world->pa_systems, 0, sizeof(system_t) * num_max_systems);

        for (size_t i = 0; i < num_max_systems; ++i)
        {
            system_t* p_system = p_world->pa_systems + i;

            if (!dynamic_vector_initialize(&p_system->archetypes, sizeof(archetype_t*), pow2_log2c1))
            {
                // 실패 전까지 초기화된 p_system 해제
                for (size_t j = 0; j < i; ++j)
                {
                    dynamic_vector_release(&p_world->pa_systems[j].archetypes);
                }

                ASSERT(false, "Failed to init systems");
                goto failed_init_systems;
            }
        }
    }

    return true;

failed_init_systems:
    SAFE_FREE(p_world->pa_systems);

failed_malloc_systems:
    map_release(&p_world->registered_systems);

failed_init_registered_system_map:
    map_release(&p_world->archetype_map);

failed_init_archetype_map:
    SAFE_FREE(p_world->pa_component_size);

failed_malloc_component_size:
    map_release(&p_world->registered_components);

failed_init_registered_component_map:
    SAFE_FREE(p_world->destroyed_entities.pa_entities);

failed_malloc_destroyed_entities_queue:
    SAFE_FREE(p_world->pa_no_archetype_entities);

failed_malloc_no_archetype_entities:
    SAFE_FREE(p_world->pa_entity_fields);

failed_malloc_entity_fields:
    chunked_memory_pool_release(&p_world->mask_pool);

failed_init_component_mask_pool:
    memset(p_world, 0, sizeof(ecs_world_t));
    return false;
}

ecs_id_t __stdcall register_component(i_ecs_t* p_this, const char* p_component_name, const size_t component_size)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_component_name != NULL, "p_component_name == NULL");
    ASSERT(component_size > 0, "component_size == 0");

    ecs_world_t* p_world = (ecs_world_t*)p_this;

    if (map_get_num_elements(&p_world->registered_components) >= p_world->num_max_components)
    {
        ASSERT(false, "num components >= num max components");
        return 0;
    }

    const size_t len = strlen(p_component_name);
    const ecs_hash_t hash = hash64_fnv1a(p_component_name, len);
    const ecs_id_t* p_component = (ecs_id_t*)map_get_value_by_hash_or_null(&p_world->registered_components, hash, &hash, sizeof(ecs_hash_t));
    if (p_component == NULL)
    {
        const ecs_id_t component = map_get_num_elements(&p_world->registered_components) | ECS_FLAG_COMPONENT;

        map_insert_by_hash(&p_world->registered_components, hash, &hash, sizeof(ecs_hash_t), &component, sizeof(ecs_id_t));
        p_world->pa_component_size[PURE_ECS_ID(component)] = component_size;

        return component;
    }

    return *p_component;
}

ecs_id_t __cdecl register_system(i_ecs_t* p_this, const char* p_system_name, ecs_system_func p_func, const size_t num_components, ...)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(num_components > 0, "num_components == 0");

    ecs_world_t* p_world = (ecs_world_t*)p_this;

    if (map_get_num_elements(&p_world->registered_systems) >= p_world->num_max_systems)
    {
        ASSERT(false, "num_systems >= num max systems");
        return 0;
    }

    const size_t len = strlen(p_system_name);
    const ecs_hash_t hash = hash64_fnv1a(p_system_name, len);
    const ecs_id_t* p_ecs_id = (ecs_id_t*)map_get_value_by_hash_or_null(&p_world->registered_systems, hash, &hash, sizeof(ecs_hash_t));
    if (p_ecs_id == NULL)
    {
        const ecs_id_t ecs_id = map_get_num_elements(&p_world->registered_systems) | ECS_FLAG_SYSTEM;

        map_insert_by_hash(&p_world->registered_systems, hash, &hash, sizeof(ecs_hash_t), &ecs_id, sizeof(ecs_id_t));

        system_t* p_system = p_world->pa_systems + PURE_ECS_ID(ecs_id);
        p_system->p_func = p_func;
        p_system->mask.num_components = num_components;
        p_system->mask.p_masks = (uint64_t*)chunked_memory_pool_alloc_or_null(&p_world->mask_pool);
        memset(p_system->mask.p_masks, 0, 8 * NUM_MASKS(p_world->num_max_components));

        va_list vl;
        va_start(vl, num_components);
        {
            for (size_t i = 0; i < num_components; ++i)
            {
                const ecs_id_t component = va_arg(vl, ecs_id_t);
                const ecs_id_t pure_component = PURE_ECS_ID(component);

                ASSERT(IS_COMPONENT(component), "Not component");

                const uint64_t index = pure_component >> 3;
                const uint64_t mask = (uint64_t)1 << pure_component;
                p_system->mask.p_masks[index] |= mask;
            }
        }
        va_end(vl);

        p_system->mask.hash = hash64_fnv1a((const char*)p_system->mask.p_masks, 8 * NUM_MASKS(p_world->num_max_components));

        return ecs_id;
    }

    return *p_ecs_id;
}

ecs_id_t __stdcall get_component_id(const i_ecs_t* p_this, const char* p_component_name)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_component_name != NULL, "p_component_name == NULL");

    ecs_world_t* p_world = (ecs_world_t*)p_this;

    const size_t len = strlen(p_component_name);
    const ecs_hash_t hash = hash64_fnv1a(p_component_name, len);
    const ecs_id_t* p_component = (ecs_id_t*)map_get_value_by_hash_or_null(&p_world->registered_components, hash, &hash, sizeof(ecs_hash_t));
    if (p_component == NULL)
    {
        return 0;
    }

    return *p_component;
}

ecs_id_t __stdcall get_system_id(const i_ecs_t* p_this, const char* p_system_name)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_system_name != NULL, "p_system_name == NULL");

    ecs_world_t* p_world = (ecs_world_t*)p_this;

    const size_t len = strlen(p_system_name);
    const ecs_hash_t hash = hash64_fnv1a(p_system_name, len);
    const ecs_id_t* p_system = (ecs_id_t*)map_get_value_by_hash_or_null(&p_world->registered_systems, hash, &hash, sizeof(ecs_hash_t));
    if (p_system == NULL)
    {
        return 0;
    }

    return *p_system;
}

ecs_id_t __stdcall create_entity(i_ecs_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    ecs_world_t* p_world = (ecs_world_t*)p_this;

    if (p_world->num_entities >= p_world->num_max_entities)
    {
        ASSERT(false, "num entities >= num_max_entities");
        return 0;
    }

    entity_queue_t* p_queue = &p_world->destroyed_entities;

    ecs_id_t entity;
    if (p_queue->front == p_queue->rear)
    {
        entity = p_world->entity_count++ | ECS_FLAG_ENTITY;
    }
    else
    {
        p_queue->front = (p_queue->front + 1) % (p_world->num_max_entities + 1);
        entity = p_queue->pa_entities[p_queue->front];
    }

    // entity field 초기화
    entity_field_t* p_field = get_entity_field_or_null(p_world, entity);
    p_field->id = entity;
    p_field->col = p_world->num_no_archetype_entities;
    p_field->p_archetype = NULL;

    p_world->pa_no_archetype_entities[p_field->col] = entity;
    ++p_world->num_no_archetype_entities;

    ++p_world->num_entities;

    return entity;
}

bool __stdcall is_alive_entity(const i_ecs_t* p_this, const ecs_id_t entity)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    ecs_world_t* p_world = (ecs_world_t*)p_this;

    const ecs_id_t pure_entity = PURE_ECS_ID(entity);
    return IS_ENTITY(entity) && p_world->pa_entity_fields[pure_entity].id == entity;
}

bool __stdcall destroy_entity(i_ecs_t* p_this, const ecs_id_t entity)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    ecs_world_t* p_world = (ecs_world_t*)p_this;

    if (!is_alive_entity(p_this, entity))
    {
        ASSERT(false, "Already destroyed or invalid entity id");
        return false;
    }

    entity_queue_t* p_queue = &p_world->destroyed_entities;
    if ((p_queue->rear + 1) % (p_world->num_max_entities + 1) == p_queue->front)
    {
        ASSERT(false, "Saturation queue");
        return false;
    }

    ecs_id_t next_entity = entity + ENTITY_GEN_FLAG;
    if (ENTITY_GEN(entity) >= MAX_ENTITY_GEN_FLAG)
    {
        next_entity = PURE_ECS_ID(entity) | ECS_FLAG_ENTITY;
    }
    else
    {
        next_entity = entity + ENTITY_GEN_FLAG;
    }

    p_queue->rear = (p_queue->rear + 1) % (p_world->num_max_entities + 1);
    p_queue->pa_entities[p_queue->rear] = next_entity;

    // archetype에서 instance 제거
    {
        entity_field_t* p_field = get_entity_field_or_null(p_world, entity);
        archetype_t* p_archetype = p_field->p_archetype;

        // instance 제거
        move_archetype_to_null(p_world, entity);

        // archetype 없는 entity 배열 수정
        const ecs_id_t last_entity = p_world->pa_no_archetype_entities[p_world->num_no_archetype_entities - 1];
        p_world->pa_no_archetype_entities[p_field->col] = last_entity;

        entity_field_t* p_last_field = get_entity_field_or_null(p_world, last_entity);
        p_last_field->col = p_field->col;

        --p_world->num_no_archetype_entities;
    }

    // field 수정
    entity_field_t* p_field = get_entity_field_or_null(p_world, entity);
    p_field->id = next_entity;

    --p_world->num_entities;

    return true;
}

bool __cdecl has_component(const i_ecs_t* p_this, const ecs_id_t entity, const size_t num_components, ...)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    ecs_world_t* p_world = (ecs_world_t*)p_this;

    entity_field_t* p_record = get_entity_field_or_null(p_world, entity);
    ASSERT(p_record != NULL, "p_field == NULL");

    archetype_t* p_archetype = p_record->p_archetype;
    if (p_archetype == NULL)
    {
        return false;
    }

    uint64_t* p_masks = p_archetype->mask.p_masks;
    ASSERT(p_masks != NULL, "p_mask == NULL");

    size_t count = 0;

    va_list vl;
    va_start(vl, num_components);
    {
        for (size_t i = 0; i < num_components; ++i)
        {
            const ecs_id_t component = va_arg(vl, ecs_id_t);
            const ecs_id_t pure_component = PURE_ECS_ID(component);

            ASSERT(IS_COMPONENT(component), "Not component");

            const uint64_t index = pure_component >> 3;
            const uint64_t mask = (uint64_t)1 << pure_component;

            // 가지고 있는 component인지 검사
            if ((p_masks[index] & mask) != 0)
            {
                ++count;
            }
        }
    }
    va_end(vl);

    return count == num_components;
}

bool __cdecl add_component(i_ecs_t* p_this, const ecs_id_t entity, const size_t num_components, ...)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(num_components > 0, "num_components == 0");

    ecs_world_t* p_world = (ecs_world_t*)p_this;

    entity_field_t* p_field = get_entity_field_or_null(p_world, entity);
    if (p_field == NULL)
    {
        ASSERT(false, "Destroyed entity or invalid entity");
        return false;
    }

    archetype_t* p_archetype = p_field->p_archetype;
    ecs_mask_t next_mask = { 0, };
    next_mask.p_masks = (uint64_t*)chunked_memory_pool_alloc_or_null(&p_world->mask_pool);

    // 기존의 component mask 복사
    if (p_archetype == NULL)
    {
        memset(next_mask.p_masks, 0, 8 * NUM_MASKS(p_world->num_max_components));
    }
    else
    {
        const ecs_mask_t* p_mask = &p_archetype->mask;
        memcpy(next_mask.p_masks, p_mask->p_masks, 8 * NUM_MASKS(p_world->num_max_components));
        next_mask.num_components = p_mask->num_components;
    }

    // component 추가
    {
        uint64_t* p_masks = next_mask.p_masks;
        ASSERT(p_masks != NULL, "p_mask == NULL");

        va_list vl;
        va_start(vl, num_components);
        {
            for (size_t i = 0; i < num_components; ++i)
            {
                const ecs_id_t component = va_arg(vl, ecs_id_t);
                const ecs_id_t pure_component = PURE_ECS_ID(component);

                ASSERT(IS_COMPONENT(component), "Not component");

                const uint64_t index = pure_component >> 3;
                const uint64_t mask = (uint64_t)1 << pure_component;

                // 추가되지 않은 component라면 개수 증가
                if ((p_masks[index] ^ mask) != 0)
                {
                    ++next_mask.num_components;
                }

                p_masks[index] |= mask;
            }
        }
        va_end(vl);
    }

    next_mask.hash = hash64_fnv1a((const char*)next_mask.p_masks, 8 * NUM_MASKS(p_world->num_max_components));
    archetype_t* p_next_archetype = (archetype_t*)map_get_value_by_hash_or_null(&p_world->archetype_map,
                                                                                next_mask.hash,
                                                                                &next_mask.hash,
                                                                                sizeof(ecs_id_t));
    if (p_next_archetype == NULL)
    {
        if (!create_archetype(p_world, &next_mask, &p_next_archetype))
        {
            ASSERT(false, "Failed to create next archetype");
            goto failed_to_create_archetype;
        }
    }
    else
    {
        chunked_memory_pool_dealloc(&p_world->mask_pool, next_mask.p_masks);
    }

    if (p_archetype == p_next_archetype)
    {
        return false;
    }

    if (p_archetype == NULL)
    {
        return move_archetype_from_null(p_world, entity, p_next_archetype);
    }

    return move_archetype(p_world, entity, p_next_archetype);

failed_to_create_archetype:
    chunked_memory_pool_dealloc(&p_world->mask_pool, next_mask.p_masks);
    return false;
}

bool __stdcall set_component(i_ecs_t* p_this, const ecs_id_t entity, const ecs_id_t component, void* p_value)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(IS_ENTITY(entity), "Not entity");
    ASSERT(IS_COMPONENT(component), "Not component");
    ASSERT(p_value != NULL, "p_value == NULL");

    ecs_world_t* p_world = (ecs_world_t*)p_this;

    if (!has_component(p_this, entity, 1, component))
    {
        add_component(p_this, entity, 1, component);
    }

    entity_field_t* p_field = get_entity_field_or_null(p_world, entity);
    if (p_field == NULL)
    {
        ASSERT(false, "Destroyed entity or invalid entity");
        return false;
    }

    archetype_t* p_archetype = p_field->p_archetype;
    const size_t component_index = *(size_t*)map_get_value_or_null(&p_archetype->component_map, &component, sizeof(ecs_id_t));
    const size_t component_size = p_world->pa_component_size[PURE_ECS_ID(component)];

    char* p_instances = dynamic_vector_get_elements_ptr_or_null(&p_archetype->pa_instances_array[component_index]);
    char* p_instance = p_instances + component_size * p_field->col;
    memcpy(p_instance, p_value, component_size);

    return true;
}

void* __stdcall get_component_or_null(const i_ecs_t* p_this, const ecs_id_t entity, const ecs_id_t component)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(IS_ENTITY(entity), "Not entity");
    ASSERT(IS_COMPONENT(component), "Not component");

    const ecs_world_t* p_world = (ecs_world_t*)p_this;

    if (!has_component(p_this, entity, 1, component))
    {
        return NULL;
    }

    entity_field_t* p_field = get_entity_field_or_null(p_world, entity);
    if (p_field == NULL)
    {
        ASSERT(false, "Destroyed entity or invalid entity");
        return false;
    }

    archetype_t* p_archetype = p_field->p_archetype;
    const size_t component_index = *(size_t*)map_get_value_or_null(&p_archetype->component_map, &component, sizeof(ecs_id_t));
    const size_t component_size = p_world->pa_component_size[PURE_ECS_ID(component)];

    char* p_instances = dynamic_vector_get_elements_ptr_or_null(&p_archetype->pa_instances_array[component_index]);
    char* p_instance = p_instances + component_size * p_field->col;

    return (void*)p_instance;
}

bool __cdecl remove_component(i_ecs_t* p_this, const ecs_id_t entity, const size_t num_components, ...)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(num_components > 0, "num_components == 0");

    ecs_world_t* p_world = (ecs_world_t*)p_this;

    entity_field_t* p_field = get_entity_field_or_null(p_world, entity);
    if (p_field == NULL)
    {
        ASSERT(false, "Destroyed entity or invalid entity");
        return false;
    }

    archetype_t* p_archetype = p_field->p_archetype;
    if (p_archetype == NULL)
    {
        return false;
    }

    ecs_mask_t next_mask = { 0, };
    next_mask.p_masks = (uint64_t*)chunked_memory_pool_alloc_or_null(&p_world->mask_pool);

    const ecs_mask_t* p_mask = &p_archetype->mask;
    memcpy(next_mask.p_masks, p_mask->p_masks, 8 * NUM_MASKS(p_world->num_max_components));
    next_mask.num_components = p_mask->num_components;

    // component 제거
    {
        uint64_t* p_masks = next_mask.p_masks;
        ASSERT(p_masks != NULL, "p_mask == NULL");

        va_list vl;
        va_start(vl, num_components);
        {
            for (size_t i = 0; i < num_components; ++i)
            {
                const ecs_id_t component = va_arg(vl, ecs_id_t);
                const ecs_id_t pure_component = PURE_ECS_ID(component);

                ASSERT(IS_COMPONENT(component), "Not component");

                const uint64_t index = pure_component >> 3;
                const uint64_t mask = (uint64_t)1 << pure_component;

                // 추가되지 않은 component라면 개수 증가
                if ((p_masks[index] ^ mask) != 0)
                {
                    --next_mask.num_components;
                }

                p_masks[index] &= ~mask;
            }
        }
        va_end(vl);
    }

    next_mask.hash = hash64_fnv1a((const char*)next_mask.p_masks, 8 * NUM_MASKS(p_world->num_max_components));
    archetype_t* p_next_archetype = (archetype_t*)map_get_value_by_hash_or_null(&p_world->archetype_map,
                                                                                next_mask.hash,
                                                                                &next_mask.hash,
                                                                                sizeof(ecs_id_t));
    if (p_next_archetype == NULL)
    {
        const size_t num_masks = NUM_MASKS(p_world->num_max_components);
        size_t count = 0;
        for (size_t i = 0; i < num_masks; ++i)
        {
            if (p_mask->p_masks[i] != 0)
            {
                break;
            }

            ++count;
        }

        if (count != num_masks && !create_archetype(p_world, &next_mask, &p_next_archetype))
        {
            ASSERT(false, "Failed to create next archetype");
            goto failed_to_create_archetype;
        }
    }
    else
    {
        chunked_memory_pool_dealloc(&p_world->mask_pool, next_mask.p_masks);
    }

    if (p_archetype == p_next_archetype)
    {
        return false;
    }

    if (p_next_archetype == NULL)
    {
        return move_archetype_to_null(p_world, entity);
    }

    return move_archetype(p_world, entity, p_next_archetype);

failed_to_create_archetype:
    chunked_memory_pool_dealloc(&p_world->mask_pool, next_mask.p_masks);
    return false;
}

bool __stdcall update_system(i_ecs_t* p_this, const ecs_id_t system)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(IS_SYSTEM(system), "Not system");

    ecs_world_t* p_world = (ecs_world_t*)p_this;
    ASSERT(PURE_ECS_ID(system) < map_get_num_elements(&p_world->registered_systems), "Invalid system");

    system_t* p_system = p_world->pa_systems + PURE_ECS_ID(system);

    ecs_view_t view = { 0, };
    view.p_this = p_this;
    view.system = system;
    view.num_archetypes = dynamic_vector_get_num_elements(&p_system->archetypes);

    p_system->p_func(&view);

    return true;
}

void* __stdcall get_instances_or_null(const ecs_view_t* p_view, const size_t archetype_index, const ecs_id_t component)
{
    ASSERT(p_view != NULL, "p_view == NULL");
    ASSERT(IS_COMPONENT(component), "Not component");

    ecs_world_t* p_world = (ecs_world_t*)p_view->p_this;

    if (archetype_index >= p_view->num_archetypes)
    {
        return NULL;
    }

    system_t* p_system = p_world->pa_systems + PURE_ECS_ID(p_view->system);

    archetype_t** pp_archetypes = (archetype_t**)dynamic_vector_get_elements_ptr_or_null(&p_system->archetypes);
    archetype_t* p_archetype = pp_archetypes[archetype_index];
    const size_t component_index = *(size_t*)map_get_value_or_null(&p_archetype->component_map, &component, sizeof(ecs_id_t));

    dynamic_vector_t* p_instances = &p_archetype->pa_instances_array[component_index];
    return dynamic_vector_get_elements_ptr_or_null(p_instances);
}

size_t __stdcall get_num_instances(const ecs_view_t* p_view, const size_t archetype_index)
{
    ASSERT(p_view != NULL, "p_view == NULL");

    if (archetype_index >= p_view->num_archetypes)
    {
        return 0;
    }

    ecs_world_t* p_world = (ecs_world_t*)p_view->p_this;

    system_t* p_system = p_world->pa_systems + PURE_ECS_ID(p_view->system);

    archetype_t** pp_archetypes = (archetype_t**)dynamic_vector_get_elements_ptr_or_null(&p_system->archetypes);
    archetype_t* p_archetype = pp_archetypes[archetype_index];

    return p_archetype->num_instances;
}

void* __stdcall get_entities_or_null(const ecs_view_t* p_view, const size_t archetype_index)
{
    ASSERT(p_view != NULL, "p_view == NULL");

    if (archetype_index >= p_view->num_archetypes)
    {
        return NULL;
    }

    ecs_world_t* p_world = (ecs_world_t*)p_view->p_this;

    system_t* p_system = p_world->pa_systems + PURE_ECS_ID(p_view->system);

    archetype_t** pp_archetypes = (archetype_t**)dynamic_vector_get_elements_ptr_or_null(&p_system->archetypes);
    archetype_t* p_archetype = pp_archetypes[archetype_index];

    return dynamic_vector_get_elements_ptr_or_null(&p_archetype->entities);
}

static bool create_archetype(ecs_world_t* p_world, const ecs_mask_t* p_mask, archetype_t** pp_out_archetype)
{
    ASSERT(p_world != NULL, "p_world == NULL");
    ASSERT(p_mask != NULL, "p_component_mask == NULL");
    ASSERT(pp_out_archetype != NULL, "pp_out_archetype == NULL");

    // map에 archetype 추가
    {
        archetype_t archetype = { 0, };
        map_insert_by_hash(&p_world->archetype_map, p_mask->hash, &p_mask->hash, sizeof(ecs_id_t), &archetype, sizeof(archetype_t));
    }

    archetype_t* p_archetype = (archetype_t*)map_get_value_by_hash_or_null(&p_world->archetype_map, p_mask->hash, &p_mask->hash, sizeof(ecs_id_t));

    // component map 초기화
    if (!map_initialize(&p_archetype->component_map, sizeof(ecs_id_t), sizeof(size_t), p_mask->num_components))
    {
        ASSERT(false, "Failed to init component map");
        goto failed_init_component_map;
    }

    // mask 복사
    p_archetype->mask = *p_mask;

    // instances array 생성
    p_archetype->pa_instances_array = (dynamic_vector_t*)malloc(sizeof(dynamic_vector_t) * p_mask->num_components);
    if (p_archetype->pa_instances_array == NULL)
    {
        ASSERT(false, "Failed to malloc instances array");
        goto failed_malloc_instances_array;
    }

    uint_t log2e1 = 0;
    log2int64(&log2e1, p_world->num_max_entities);
    ++log2e1;
    const size_t pow2_log2e1 = (size_t)log2e1 * log2e1;

    // entities 초기화
    if (!dynamic_vector_initialize(&p_archetype->entities, sizeof(ecs_id_t), pow2_log2e1))
    {
        ASSERT(false, "Failed to init entities");
        goto failed_init_entities;
    }

    // instance vector 초기화
    size_t component_index = 0;
    dynamic_vector_t* p_instances = p_archetype->pa_instances_array;

    const size_t num_masks = NUM_MASKS(p_world->num_max_components);
    for (size_t i = 0; i < num_masks; ++i)
    {
        uint64_t mask = p_mask->p_masks[i];
        uint_t msb_index = 0;
        while (log2int64(&msb_index, mask))
        {
            const uint64_t msb_mask = (uint64_t)1 << msb_index;
            const ecs_id_t component = msb_index | ECS_FLAG_COMPONENT;

            const size_t component_size = p_world->pa_component_size[PURE_ECS_ID(component)];

            if (!dynamic_vector_initialize(p_instances, component_size, pow2_log2e1))
            {
                // 복구
                dynamic_vector_t* p_temp_instances = p_archetype->pa_instances_array;
                while (p_temp_instances != p_instances)
                {
                    dynamic_vector_release(p_temp_instances);
                }

                ASSERT(false, "Failed to init instance vector");
                goto failed_init_instances;
            }

            ++p_instances;

            // component map 삽입
            map_insert(&p_archetype->component_map, &component, sizeof(ecs_id_t), &component_index, sizeof(size_t));
            ++component_index;

            mask ^= msb_mask;
        }
    }

    *pp_out_archetype = p_archetype;
    ++p_world->num_archetypes;

    // system
    for (size_t i = 0; i < map_get_num_elements(&p_world->registered_systems); ++i)
    {
        system_t* p_system = p_world->pa_systems + i;

        for (size_t j = 0; j < num_masks; ++j)
        {
            const uint64_t mask = p_system->mask.p_masks[j] & p_archetype->mask.p_masks[j];
            if (mask > 0 && mask == p_system->mask.p_masks[j])
            {
                dynamic_vector_push_back(&p_system->archetypes, &p_archetype, sizeof(archetype_t*));
            }
        }
    }

    return true;

failed_init_instances:
    dynamic_vector_release(&p_archetype->entities);

failed_init_entities:
    SAFE_FREE(p_archetype->pa_instances_array);

failed_malloc_instances_array:
    map_release(&p_archetype->component_map);

failed_init_component_map:
    map_remove_by_hash(&p_world->archetype_map, p_mask->hash, &p_mask->hash, sizeof(ecs_id_t));
    *pp_out_archetype = NULL;
    return false;
}

static FORCEINLINE entity_field_t* get_entity_field_or_null(const ecs_world_t* p_world, const ecs_id_t entity)
{
    ASSERT(p_world != NULL, "p_world == NULL");

    if (!is_alive_entity(&p_world->base, entity))
    {
        ASSERT(false, "Invalid entity id");
        return NULL;
    }

    return &p_world->pa_entity_fields[PURE_ECS_ID(entity)];
}

static bool move_archetype(ecs_world_t* p_world, const ecs_id_t entity, archetype_t* p_to_archetype)
{
    ASSERT(p_world != NULL, "p_world == NULL");
    ASSERT(p_to_archetype != NULL, "p_to_archetype == NULL");
    ASSERT(is_alive_entity(&p_world->base, entity), "Invalid entity");

    entity_field_t* p_field = get_entity_field_or_null(p_world, entity);
    archetype_t* p_from_archetype = p_field->p_archetype;

    if (p_from_archetype == NULL || p_to_archetype == NULL)
    {
        return false;
    }

    const size_t from_col = p_field->col;
    const size_t to_col = p_to_archetype->num_instances;

    dynamic_vector_t* p_from_instances_array = p_from_archetype->pa_instances_array;
    dynamic_vector_t* p_to_instances_array = p_to_archetype->pa_instances_array;

    // 인스턴스 이동
    const size_t num_masks = NUM_MASKS(p_world->num_max_components);
    for (size_t i = 0; i < num_masks; ++i)
    {
        const uint64_t from_mask = p_from_archetype->mask.p_masks[i];
        const uint64_t to_mask = p_to_archetype->mask.p_masks[i];

        const uint64_t xor_mask = from_mask ^ to_mask;

        const uint64_t move_mask = from_mask & to_mask;
        const uint64_t add_mask = to_mask & xor_mask;
        const uint64_t remove_mask = from_mask & xor_mask;

        uint_t msb_index = 0;

        // 이동
        uint64_t mask = move_mask;
        while (log2int64(&msb_index, mask))
        {
            const uint64_t msb_mask = (uint64_t)1 << msb_index;
            const ecs_id_t component = ((uint64_t)msb_index + (uint64_t)64 * i) | ECS_FLAG_COMPONENT;

            const size_t from_row = *(size_t*)map_get_value_or_null(&p_from_archetype->component_map,
                                                                    &component,
                                                                    sizeof(ecs_id_t));
            const size_t to_row = *(size_t*)map_get_value_or_null(&p_to_archetype->component_map,
                                                                  &component,
                                                                  sizeof(ecs_id_t));

            const size_t component_size = p_world->pa_component_size[PURE_ECS_ID(component)];

            dynamic_vector_t* p_from_instances = &p_from_instances_array[from_row];
            dynamic_vector_t* p_to_instances = &p_to_instances_array[to_row];
            dynamic_vector_push_back_empty(p_to_instances);

            char* p_from_instance = (char*)dynamic_vector_get_element_or_null(p_from_instances, from_col);
            char* p_from_last_instance = (char*)dynamic_vector_get_back_or_null(p_from_instances);
            char* p_to_instance = (char*)dynamic_vector_get_back_or_null(p_to_instances);

            memcpy(p_to_instance, p_from_instance, component_size);
            memcpy(p_from_instance, p_from_last_instance, component_size);

            dynamic_vector_pop_back(p_from_instances);

            mask ^= msb_mask;
        }

        // 추가
        mask = add_mask;
        while (log2int64(&msb_index, mask))
        {
            const uint64_t msb_mask = (uint64_t)1 << msb_index;
            const ecs_id_t component = ((uint64_t)msb_index + (uint64_t)64 * i) | ECS_FLAG_COMPONENT;

            const size_t to_row = *(size_t*)map_get_value_or_null(&p_to_archetype->component_map,
                                                                  &component,
                                                                  sizeof(ecs_id_t));

            dynamic_vector_t* p_to_instances = &p_to_instances_array[to_row];
            dynamic_vector_push_back_empty(p_to_instances);

            mask ^= msb_mask;
        }

        // 삭제
        mask = remove_mask;
        while (log2int64(&msb_index, mask))
        {
            const uint64_t msb_mask = (uint64_t)1 << msb_index;
            const ecs_id_t component = ((uint64_t)msb_index + (uint64_t)64 * i) | ECS_FLAG_COMPONENT;

            const size_t from_row = *(size_t*)map_get_value_or_null(&p_from_archetype->component_map,
                                                                    &component,
                                                                    sizeof(ecs_id_t));

            const size_t component_size = p_world->pa_component_size[PURE_ECS_ID(component)];

            dynamic_vector_t* p_from_instances = &p_from_instances_array[from_row];

            char* p_from_instance = (char*)dynamic_vector_get_element_or_null(p_from_instances, from_col);
            char* p_from_last_instance = (char*)dynamic_vector_get_back_or_null(p_from_instances);
            memcpy(p_from_instance, p_from_last_instance, component_size);

            dynamic_vector_pop_back(p_from_instances);

            mask ^= msb_mask;
        }
    }

    // to archetype에 entity 추가
    dynamic_vector_push_back(&p_to_archetype->entities, &entity, sizeof(ecs_id_t));

    // from archetype에서 entity 제거
    ecs_id_t* p_from_entity = (ecs_id_t*)dynamic_vector_get_element_or_null(&p_from_archetype->entities, from_col);
    const ecs_id_t* p_from_last_entity = (ecs_id_t*)dynamic_vector_get_back_or_null(&p_from_archetype->entities);
    *p_from_entity = *p_from_last_entity;
    dynamic_vector_pop_back(&p_from_archetype->entities);

    // from archetype에서 field 수정
    entity_field_t* p_from_last_field = get_entity_field_or_null(p_world, *p_from_last_entity);
    p_from_last_field->col = from_col;

    // 이동 후 entity field 수정
    {
        p_field->p_archetype = p_to_archetype;
        p_field->col = to_col;
    }

    --p_from_archetype->num_instances;
    ++p_to_archetype->num_instances;

    return true;
}

static bool move_archetype_from_null(ecs_world_t* p_world, const ecs_id_t entity, archetype_t* p_to_archetype)
{
    ASSERT(p_world != NULL, "p_world == NULL");
    ASSERT(is_alive_entity(&p_world->base, entity), "Invalid entity");

    if (p_to_archetype == NULL)
    {
        return false;
    }

    entity_field_t* p_field = get_entity_field_or_null(p_world, entity);

    const size_t to_col = p_to_archetype->num_instances;

    dynamic_vector_t* p_to_instances_array = p_to_archetype->pa_instances_array;

    // 인스턴스 이동
    const size_t num_masks = NUM_MASKS(p_world->num_max_components);
    for (size_t i = 0; i < num_masks; ++i)
    {
        uint64_t mask = p_to_archetype->mask.p_masks[i];
        uint_t msb_index = 0;

        // 추가
        while (log2int64(&msb_index, mask))
        {
            const uint64_t msb_mask = (uint64_t)1 << msb_index;
            const ecs_id_t component = ((uint64_t)msb_index + (uint64_t)64 * i) | ECS_FLAG_COMPONENT;

            const size_t to_row = *(size_t*)map_get_value_or_null(&p_to_archetype->component_map,
                                                                  &component,
                                                                  sizeof(ecs_id_t));

            dynamic_vector_t* p_to_instances = &p_to_instances_array[to_row];
            dynamic_vector_push_back_empty(p_to_instances);

            mask ^= msb_mask;
        }
    }

    // to archetype에 entity 추가
    dynamic_vector_push_back(&p_to_archetype->entities, &entity, sizeof(ecs_id_t));

    // entity field 수정
    {
        // last no archetype field 수정
        const ecs_id_t last_no_archetype_entity = p_world->pa_no_archetype_entities[p_world->num_no_archetype_entities - 1];
        entity_field_t* p_last_no_archetype_field = get_entity_field_or_null(p_world, last_no_archetype_entity);
        p_last_no_archetype_field->col = p_field->col;

        p_world->pa_no_archetype_entities[p_field->col] = p_world->pa_no_archetype_entities[p_world->num_no_archetype_entities - 1];
        --p_world->num_no_archetype_entities;

        p_field->p_archetype = p_to_archetype;
        p_field->col = to_col;
    }

    ++p_to_archetype->num_instances;

    return true;
}

static bool move_archetype_to_null(ecs_world_t* p_world, const ecs_id_t entity)
{
    ASSERT(p_world != NULL, "p_world == NULL");
    ASSERT(is_alive_entity(&p_world->base, entity), "Invalid entity");

    entity_field_t* p_field = get_entity_field_or_null(p_world, entity);
    archetype_t* p_from_archetype = p_field->p_archetype;

    if (p_from_archetype == NULL)
    {
        return false;
    }

    const size_t from_col = p_field->col;

    dynamic_vector_t* p_from_instances_array = p_from_archetype->pa_instances_array;

    // 인스턴스 이동
    const size_t num_masks = NUM_MASKS(p_world->num_max_components);
    for (size_t i = 0; i < num_masks; ++i)
    {
        uint64_t mask = p_from_archetype->mask.p_masks[i];
        uint_t msb_index = 0;

        // 삭제
        while (log2int64(&msb_index, mask))
        {
            const uint64_t msb_mask = (uint64_t)1 << msb_index;
            const ecs_id_t component = ((uint64_t)msb_index + (uint64_t)64 * i) | ECS_FLAG_COMPONENT;

            const size_t from_row = *(size_t*)map_get_value_or_null(&p_from_archetype->component_map,
                                                                    &component,
                                                                    sizeof(ecs_id_t));

            const size_t component_size = p_world->pa_component_size[PURE_ECS_ID(component)];

            dynamic_vector_t* p_from_instances = &p_from_instances_array[from_row];

            char* p_from_instance = (char*)dynamic_vector_get_element_or_null(p_from_instances, from_col);
            char* p_from_last_instance = (char*)dynamic_vector_get_back_or_null(p_from_instances);
            memcpy(p_from_instance, p_from_last_instance, component_size);

            dynamic_vector_pop_back(p_from_instances);

            mask ^= msb_mask;
        }
    }

    // from archetype에서 entity 제거
    ecs_id_t* p_from_entity = (ecs_id_t*)dynamic_vector_get_element_or_null(&p_from_archetype->entities, from_col);
    const ecs_id_t* p_from_last_entity = (ecs_id_t*)dynamic_vector_get_back_or_null(&p_from_archetype->entities);
    *p_from_entity = *p_from_last_entity;
    dynamic_vector_pop_back(&p_from_archetype->entities);

    // from archetype에서 field 수정
    entity_field_t* p_from_last_field = get_entity_field_or_null(p_world, *p_from_last_entity);
    p_from_last_field->col = from_col;

    // entity field 수정
    {
        p_field->p_archetype = NULL;
        p_field->col = p_world->num_no_archetype_entities;

        p_world->pa_no_archetype_entities[p_world->num_no_archetype_entities] = p_field->id;
        ++p_world->num_no_archetype_entities;
    }

    --p_from_archetype->num_instances;

    return true;
}

void __stdcall create_instance(void** pp_out_instance)
{
    ASSERT(pp_out_instance != NULL, "pp_out_instance == NULL");

    static i_ecs_vtbl_t vtbl =
    {
        add_ref,
        release,
        get_ref_count,

        initialize,

        register_component,
        register_system,

        get_component_id,
        get_system_id,

        create_entity,
        is_alive_entity,
        destroy_entity,

        has_component,
        add_component,
        remove_component,
        set_component,
        get_component_or_null,

        update_system,

        get_num_instances,
        get_instances_or_null,
        get_entities_or_null
    };

    ecs_world_t* pa_world = malloc(sizeof(ecs_world_t));
    if (pa_world == NULL)
    {
        ASSERT(false, "Failed to malloc world");
        *pp_out_instance = NULL;
        return;
    }

    pa_world->base.vtbl = &vtbl;
    pa_world->ref_count = 1;

    *pp_out_instance = pa_world;
}