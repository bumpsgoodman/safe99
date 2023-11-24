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
    get_sin_cos(-p_transform->rotation.z * (float)PI_DIV_180, &sr, &cr);

#if 1
    p_transform->right_vector.x = cy * cr + sy * sp * sr;
    p_transform->right_vector.y = cy * -sr + sy * sp * cr;
    p_transform->right_vector.z = sy * cp;

    p_transform->up_vector.x = cp * sr;
    p_transform->up_vector.y = cp * cr;
    p_transform->up_vector.z = -sp;

    p_transform->forward_vector.x = -sy * cr + cy * sp * sr;
    p_transform->forward_vector.y = sp * sr + cy * sp * cr;
    p_transform->forward_vector.z = cy * cp;
#endif

#if 0
    p_transform->right_vector.x = cy * cr + sy * sp * sr;
    p_transform->right_vector.y = cp * sr;
    p_transform->right_vector.z = -sy * cr + cy * sp * sr;

    p_transform->up_vector.x = cy * -sr + sy * sp * cr;
    p_transform->up_vector.y = cp * cr;
    p_transform->up_vector.z = sp * sr + cy * sp * cr;

    p_transform->forward_vector.x = sy * cp;
    p_transform->forward_vector.y = -sp;
    p_transform->forward_vector.z = cy * cp;
#endif
}

FORCEINLINE void __stdcall transform3_set_position(transform3_t* p_transform, const vector3_t position)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->position = position;
}

FORCEINLINE void __stdcall transform3_set_rotation(transform3_t* p_transform, const vector3_t rotation)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    p_transform->rotation = vector_to_vector3(vector_wrap_scalar(vector3_to_vector(&rotation), 0.0f, 360.0f));
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

    const vector3_t r = p_transform->right_vector;
    const vector3_t u = p_transform->up_vector;
    const vector3_t f = p_transform->forward_vector;
    const vector3_t t = p_transform->position;
    const vector3_t s = p_transform->scale;

#if 1
    matrix_t result;
    result.r0 = vector_mul_scalar(vector3_to_vector(&r), s.x);
    result.r1 = vector_mul_scalar(vector3_to_vector(&u), s.y);
    result.r2 = vector_mul_scalar(vector3_to_vector(&f), s.z);
    result.r3 = vector3_to_vector(&t);
#endif

#if 0
    matrix_t result = matrix_set(r.x * s.x, u.x * s.y, f.x * s.z, t.x,
                                 r.y * s.x, u.y * s.y, f.y * s.z, t.y,
                                 r.z * s.x, u.z * s.y, f.z * s.z, t.z,
                                 0.0f, 0.0f, 0.0f, 1.0f);
#endif
    return result;
}

FORCEINLINE matrix_t __stdcall transform3_get_view_matrix(const transform3_t* p_transform)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");

    const vector3_t r = p_transform->right_vector;
    const vector3_t u = p_transform->up_vector;
    const vector3_t f = p_transform->forward_vector;
    const vector3_t t = p_transform->position;
    const vector_t tv = vector3_to_vector(&t);

    const matrix_t result = matrix_set(r.x, r.y, r.z, -vector_dot3(vector3_to_vector(&r), tv),
                                       u.x, u.y, u.z, -vector_dot3(vector3_to_vector(&u), tv),
                                       f.x, f.y, f.z, -vector_dot3(vector3_to_vector(&f), tv),
                                       0.0f, 0.0f, 0.0f, 1.0f);
    return result;
}

FORCEINLINE void __stdcall transform3_add_position(transform3_t* p_transform, const vector3_t position)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    const vector_t origin = vector3_to_vector(&p_transform->position);
    const vector_t add = vector3_to_vector(&position);
    *((__m128*) & p_transform->position) = _mm_add_ps(origin, add);
}

FORCEINLINE void __stdcall transform3_add_rotation(transform3_t* p_transform, const vector3_t rotation)
{
    ASSERT(p_transform != NULL, "p_transform == NULL");
    *((__m128*) & p_transform->rotation) = _mm_add_ps(vector3_to_vector(&p_transform->rotation), vector3_to_vector(&rotation));
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
    *((__m128*) & p_transform->scale) = _mm_add_ps(vector3_to_vector(&p_transform->scale), vector3_to_vector(&scale));
}

END_EXTERN_C

#endif // TRANSFORM3_H