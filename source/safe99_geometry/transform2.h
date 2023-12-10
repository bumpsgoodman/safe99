//***************************************************************************
// 
// 파일: transform2.h
// 
// 설명: 2D 트랜스폼
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/11/17
// 
//***************************************************************************

#ifndef TRANSFORM2_H
#define TRANSFORM2_H

#include "safe99_common/defines.h"
#include "safe99_math/math.h"

START_EXTERN_C

FORCEINLINE void __stdcall transform2_update_dir_vector(transform2_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");

    float sin;
    float cos;
    get_sin_cos(p_transform->rotation * (float)PI_DIV_180, &sin, &cos);

    p_transform->right_vector.x = cos;
    p_transform->right_vector.y = -sin;

    p_transform->up_vector.x = sin;
    p_transform->up_vector.y = cos;
}

FORCEINLINE void __stdcall transform2_set_position(transform2_t* p_transform, const vector2_t position)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->position = position;
}

FORCEINLINE void __stdcall transform2_set_rotation(transform2_t* p_transform, const float degree)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->rotation = degree;
    transform2_update_dir_vector(p_transform);
}

FORCEINLINE void __stdcall transform2_set_scale(transform2_t* p_transform, const vector2_t scale)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->scale = scale;
}

FORCEINLINE vector2_t __stdcall transform2_get_position(const transform2_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    return p_transform->position;
}

FORCEINLINE float __stdcall transform2_get_rotation(const transform2_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    return p_transform->rotation;
}

FORCEINLINE vector2_t __stdcall transform2_get_scale(const transform2_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    return p_transform->scale;
}

FORCEINLINE vector2_t __stdcall transform2_get_right_vector(const transform2_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    return p_transform->right_vector;
}

FORCEINLINE vector2_t __stdcall transform2_get_up_vector(const transform2_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    return p_transform->up_vector;
}

FORCEINLINE matrix_t __stdcall transform2_get_model_matrix(const transform2_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");

    matrix_t result;
    result.r0 = vector_set(p_transform->scale.x * p_transform->right_vector.x,
                           p_transform->scale.y * p_transform->up_vector.x,
                           p_transform->position.x,
                           0.0f);
    result.r1 = vector_set(p_transform->scale.x * p_transform->right_vector.y,
                           p_transform->scale.y * p_transform->up_vector.y,
                           p_transform->position.y,
                           0.0f);
    result.r2 = s_identity_r2;
    result.r3 = s_identity_r3;
    return result;
}

FORCEINLINE void __stdcall transform2_add_position(transform2_t* p_transform, const vector2_t position)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    *((__m128*)&p_transform->position) = _mm_add_ps(TO_VECTOR(p_transform->position), TO_VECTOR(position));
}

FORCEINLINE void __stdcall transform2_add_rotation(transform2_t* p_transform, const float degree)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->rotation += degree;
    transform2_update_dir_vector(p_transform);
}

FORCEINLINE void __stdcall transform2_add_scale(transform2_t* p_transform, const vector2_t scale)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    *((__m128*)&p_transform->scale) = _mm_add_ps(TO_VECTOR(p_transform->scale), TO_VECTOR(scale));
}

END_EXTERN_C

#endif // TRANSFORM2_H