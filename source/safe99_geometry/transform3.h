//***************************************************************************
// 
// 파일: transform3.h
// 
// 설명: 3D 트랜스폼
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/11/17
// 
//***************************************************************************

#ifndef TRANSFORM3_H
#define TRANSFORM3_H

#include "safe99_common/defines.h"
#include "safe99_math/math.h"

START_EXTERN_C

FORCEINLINE void __stdcall transform3_update_dir_vector(transform3_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");

    float sy;   // sin(yaw)
    float cy;   // cos(yaw)
    float sp;   // sin(pitch)
    float cp;   // cos(pitch)
    float sr;   // sin(roll)
    float cr;   // cos(roll)
    get_sin_cos(p_transform->rotation.y * (float)PI_DIV_180, &sy, &cy);
    get_sin_cos(p_transform->rotation.x * (float)PI_DIV_180, &sp, &cp);
    get_sin_cos(p_transform->rotation.z * (float)PI_DIV_180, &sr, &cr);

    p_transform->right_vector = (vector3_t)
    {
        cy * cr + -sy * -sp * -sr,
        cy * sr + -sy * -sp * cr,
        -sy * cp
    };

    p_transform->up_vector = (vector3_t)
    {
        cp * -sr,
        cp * cr,
        sp
    };

    p_transform->forward_vector = (vector3_t)
    {
        sy * cr + cy * -sp * -sr,
        sy * sr + cy * -sp * cr,
        cy * cp
    };
}

FORCEINLINE void __stdcall transform3_set_position(transform3_t* p_transform, const vector3_t position)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->position = position;
}

FORCEINLINE void __stdcall transform3_set_rotation(transform3_t* p_transform, const vector3_t rotation)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->rotation = vector_to_vector3(vector_wrap_scalar(vector3_to_vector(rotation), 0.0f, 360.0f));
    transform3_update_dir_vector(p_transform);
}

FORCEINLINE void __stdcall transform3_set_scale(transform3_t* p_transform, const vector3_t scale)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->scale = vector_to_vector3(vector_wrap_scalar(vector3_to_vector(scale), 1.0f, 10000.0f));
}

FORCEINLINE vector3_t __stdcall transform3_get_position(const transform3_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    return p_transform->position;
}

FORCEINLINE vector3_t __stdcall transform3_get_rotation(const transform3_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    return p_transform->rotation;
}

FORCEINLINE vector3_t __stdcall transform3_get_scale(const transform3_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    return p_transform->scale;
}

FORCEINLINE void __stdcall transform3_set_right_vector(transform3_t* p_transform, const vector3_t right_vector)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->right_vector = right_vector;
}

FORCEINLINE void __stdcall transform3_set_up_vector(transform3_t* p_transform, const vector3_t up_vector)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->up_vector = up_vector;
}

FORCEINLINE void __stdcall transform3_set_forward_vector(transform3_t* p_transform, const vector3_t forward_vector)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->forward_vector = forward_vector;
}

FORCEINLINE vector3_t __stdcall transform3_get_right_vector(const transform3_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    return p_transform->right_vector;
}

FORCEINLINE vector3_t __stdcall transform3_get_up_vector(const transform3_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    return p_transform->up_vector;
}

FORCEINLINE vector3_t __stdcall transform3_get_forward_vector(const transform3_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    return p_transform->forward_vector;
}

FORCEINLINE matrix_t __stdcall transform3_get_world_matrix(const transform3_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");

    vector3_t s = p_transform->scale;

    matrix_t result;
    result.r0 = vector_mul_scalar(vector3_to_vector_zero(p_transform->right_vector), s.x);
    result.r1 = vector_mul_scalar(vector3_to_vector_zero(p_transform->up_vector), s.y);
    result.r2 = vector_mul_scalar(vector3_to_vector_zero(p_transform->forward_vector), s.z);
    result.r3 = vector3_to_vector(p_transform->position);
    return result;
}

FORCEINLINE void __stdcall transform3_add_position(transform3_t* p_transform, const vector3_t position)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    const vector_t origin = vector3_to_vector(p_transform->position);
    const vector_t add = vector3_to_vector(position);
    *((__m128*)&p_transform->position) = vector_add(origin, add);
}

FORCEINLINE void __stdcall transform3_add_rotation(transform3_t* p_transform, const vector3_t rotation)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    const vector_t origin = vector3_to_vector(p_transform->rotation);
    const vector_t add = vector3_to_vector(rotation);
    *((__m128*)&p_transform->rotation) = vector_wrap_scalar(vector_add(origin, add), 0.0f, 360.0f);
    transform3_update_dir_vector(p_transform);
}

FORCEINLINE void __stdcall transform3_add_rotation_pitch(transform3_t* p_transform, const float degree)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->rotation.x = wrap(p_transform->rotation.x + degree, 0.0f, 360.0f);
    transform3_update_dir_vector(p_transform);
}

FORCEINLINE void __stdcall transform3_add_rotation_yaw(transform3_t* p_transform, const float degree)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->rotation.y = wrap(p_transform->rotation.y + degree, 0.0f, 360.0f);
    transform3_update_dir_vector(p_transform);
}

FORCEINLINE void __stdcall transform3_add_rotation_roll(transform3_t* p_transform, const float degree)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->rotation.z = wrap(p_transform->rotation.z + degree, 0.0f, 360.0f);
    transform3_update_dir_vector(p_transform);
}

FORCEINLINE void __stdcall transform3_add_scale(transform3_t* p_transform, const vector3_t scale)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    *((__m128*)&p_transform->scale) = vector_clamp_scalar(_mm_add_ps(vector3_to_vector(p_transform->scale), vector3_to_vector(scale)), 1.0f, 10000.0f);
}

END_EXTERN_C

#endif // TRANSFORM3_H