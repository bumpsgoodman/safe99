//***************************************************************************
// 
// 파일: i_ecs.h
// 
// 설명: safe99 ecs 인터페이스
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/10/11
// 
// id 구성, 총 64비트
// -----------------------------------------------------------------
// |   8   |       16        |          16          |      24      |
// ----------------------------------------------------------------|
// |  flag |      free       |   entity_generation  |  entity_id   |
// -----------------------------------------------------------------
// 
// 함수 반환 값
// ecs_id_t 반환 함수: 실패 시 0 반환
// 
//***************************************************************************

#ifndef I_ECS_H
#define I_ECS_H

#include "safe99_common/defines.h"

#define ECS_FLAG_ENTITY 0x100000000000000ull
#define ECS_FLAG_COMPONENT 0x200000000000000ull
#define ECS_FLAG_SYSTEM 0x400000000000000ull
#define ECS_FLAG_RESERVED1 0x1000000000000000ull
#define ECS_FLAG_RESERVED2 0x2000000000000000ull
#define ECS_FLAG_RESERVED3 0x4000000000000000ull
#define ECS_FLAG_DISABLE 0x8000000000000000ull

// ecs 타입 정의
// -----------------------------------------------------------------

typedef uint64_t ecs_id_t;
typedef uint64_t ecs_hash_t;

typedef struct ecs_view ecs_view_t;
typedef void (*ecs_system_func)(const ecs_view_t*);

// -----------------------------------------------------------------

typedef interface i_ecs i_ecs_t;

typedef struct ecs_view
{
    const i_ecs_t* p_this;
    ecs_id_t system;
    size_t num_archetypes;
} ecs_view_t;

typedef interface i_ecs_vtbl
{
    size_t      (__stdcall*     add_ref)(i_ecs_t* p_this);
    size_t      (__stdcall*     release)(i_ecs_t* p_this);
    size_t      (__stdcall*     get_ref_count)(const i_ecs_t* p_this);

    bool        (__stdcall*     initialize)(i_ecs_t* p_this,
                                            const size_t num_max_entities,
                                            const size_t num_max_components,
                                            const size_t num_max_systems);

    ecs_id_t    (__stdcall*     register_component)(i_ecs_t* p_this,
                                                    const char* p_component_name,
                                                    const size_t component_size);
    ecs_id_t    (__cdecl*       register_system)(i_ecs_t* p_this,
                                                 const char* p_system_name,
                                                 ecs_system_func p_func,
                                                 const size_t num_components,
                                                 ...);

    ecs_id_t    (__stdcall*     get_component_id)(const i_ecs_t* p_this, const char* p_component_name);
    ecs_id_t    (__stdcall*     get_system_id)(const i_ecs_t* p_this, const char* p_system_name);

    ecs_id_t    (__stdcall*     create_entity)(i_ecs_t* p_this);
    bool        (__stdcall*     is_alive_entity)(const i_ecs_t* p_this, const ecs_id_t entity);
    bool        (__stdcall*     destroy_entity)(i_ecs_t* p_this, const ecs_id_t entity);

    bool        (__cdecl*       has_component)(const i_ecs_t* p_this, const ecs_id_t entity, const size_t num_components, ...);
    bool        (__cdecl*       add_component)(i_ecs_t* p_this, const ecs_id_t entity, const size_t num_components, ...);
    bool        (__cdecl*       remove_component)(i_ecs_t* p_this, const ecs_id_t entity, const size_t num_components, ...);
    bool        (__stdcall*     set_component)(i_ecs_t* p_this, const ecs_id_t entity, const ecs_id_t component, void* p_value);
    void*       (__stdcall*     get_component_or_null)(const i_ecs_t* p_this, const ecs_id_t entity, const ecs_id_t component);

    bool        (__stdcall*     update_system)(i_ecs_t* p_this, const ecs_id_t system);

    size_t      (__stdcall*     get_num_instances)(const ecs_view_t* p_view, const size_t index);
    void*       (__stdcall*     get_instances_or_null)(const ecs_view_t* p_view, const size_t index, const ecs_id_t component);
    void*       (__stdcall*     get_entities_or_null)(const ecs_view_t* p_view, const size_t index);
} i_ecs_vtbl_t;

typedef interface i_ecs
{
    i_ecs_vtbl_t* vtbl;
} i_ecs_t;

#define ECS_REGISTER_COMPONENT(p_this, component) \
    p_this->vtbl->register_component((p_this), TO_STR(component), sizeof(component))

#define ECS_REGISTER_SYSTEM(p_this, system, num_components, ...) \
    p_this->vtbl->register_system((p_this), TO_STR(system), system, (num_components), __VA_ARGS__)

#define ECS_GET_COMPONENT_ID(p_this, component) \
    p_this->vtbl->get_component_id((p_this), TO_STR(component))

#define ECS_GET_SYSTEM_ID(p_this, system) \
    p_this->vtbl->get_system_id((p_this), TO_STR(system))

#endif // I_ECS_H