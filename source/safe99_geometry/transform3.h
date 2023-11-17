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

    p_transform->right_vector.x = cy * cr + sy * sp * sr;
    p_transform->right_vector.y = cp * sr;
    p_transform->right_vector.z = -sp * cr + cy * sp * sr;

    p_transform->up_vector.x = cy * -sr + sy * sp * cr;
    p_transform->up_vector.y = cp * cr;
    p_transform->up_vector.z = sp * sr + cy * sp * cr;

    p_transform->forward_vector.x = sy * cp;
    p_transform->forward_vector.y = -sp;
    p_transform->forward_vector.z = cy * cp;
}

FORCEINLINE void __stdcall transform3_set_position(transform3_t* p_transform, const vector3_t position)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->position = position;
}

FORCEINLINE void __stdcall transform3_set_rotation(transform3_t* p_transform, const vector3_t rotation)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->rotation = rotation;
    transform3_update_dir_vector(p_transform);
}

FORCEINLINE void __stdcall transform3_set_scale(transform3_t* p_transform, const vector3_t scale)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->scale = scale;
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

FORCEINLINE matrix_t __stdcall transform3_get_model_matrix(const transform3_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");

    matrix_t result;
    const vector_t right = TO_VECTOR(p_transform->right_vector);
    const vector_t scale = TO_VECTOR(p_transform->scale);
    const vector_t rs = vector_mul(right, scale);
    result.r0 = rs;
    result.r1 = vector_set_w(vector_mul(TO_VECTOR(p_transform->up_vector), TO_VECTOR(p_transform->scale)), p_transform->position.y);
    result.r2 = vector_set_w(vector_mul(TO_VECTOR(p_transform->forward_vector), TO_VECTOR(p_transform->scale)), p_transform->position.z);
    result.r3 = s_identity_r3;
    return result;
}

FORCEINLINE void __stdcall transform3_add_position(transform3_t* p_transform, const vector3_t position)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    *((__m128*)&p_transform->position) = _mm_add_ps(TO_VECTOR(p_transform->position), TO_VECTOR(position));
}

FORCEINLINE void __stdcall transform3_add_rotation(transform3_t* p_transform, const vector3_t rotation)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    *((__m128*)&p_transform->rotation) = _mm_add_ps(TO_VECTOR(p_transform->rotation), TO_VECTOR(rotation));
    transform3_update_dir_vector(p_transform);
}

FORCEINLINE void __stdcall transform3_add_rotation_pitch(transform3_t* p_transform, const float degree)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->rotation.x += degree;
    transform3_update_dir_vector(p_transform);
}

FORCEINLINE void __stdcall transform3_add_rotation_yaw(transform3_t* p_transform, const float degree)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->rotation.y += degree;
    transform3_update_dir_vector(p_transform);
}

FORCEINLINE void __stdcall transform3_add_rotation_roll(transform3_t* p_transform, const float degree)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->rotation.z += degree;
    transform3_update_dir_vector(p_transform);
}

FORCEINLINE void __stdcall transform3_add_scale(transform3_t* p_transform, const vector3_t scale)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    *((__m128*)&p_transform->scale) = _mm_add_ps(TO_VECTOR(p_transform->scale), TO_VECTOR(scale));
}

END_EXTERN_C

#endif // TRANSFORM3_H