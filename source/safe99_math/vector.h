//***************************************************************************
// 
// 파일: vector.h
// 
// 설명: SSE로 작성한 vector
//       모든 벡터는 16바이트 정렬
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/09/25
// 
//***************************************************************************

#ifndef VECTOR_H
#define VECTOR_H

#include <intrin.h>

#include "safe99_common/defines.h"

#define TO_VECTOR(v) (*(vector_t*)(&v))
#define TO_VECTOR2(v) (*(vector2_t*)(&v))
#define TO_VECTOR3(v) (*(vector3_t*)(&v))
#define TO_VECTOR4(v) (*(vector4_t*)(&v))

START_EXTERN_C

FORCEINLINE vector_t __vectorcall vector_set(const float x, const float y, const float z, const float w)
{
    return _mm_set_ps(w, z, y, x);
}

FORCEINLINE vector_t __vectorcall vector_get_zero(void)
{
    return _mm_setzero_ps();
}

FORCEINLINE vector_t __vectorcall vector_add(const vector_t v0, const vector_t v1)
{
    return _mm_add_ps(v0, v1);
}

FORCEINLINE vector_t __vectorcall vector_sub(const vector_t v0, const vector_t v1)
{
    return _mm_sub_ps(v0, v1);
}

FORCEINLINE vector_t __vectorcall vector_mul(const vector_t v0, const vector_t v1)
{
    return _mm_mul_ps(v0, v1);
}

FORCEINLINE vector_t __vectorcall vector_mul_scalar(const vector_t v, const float scalar)
{
    return _mm_mul_ps(v, _mm_set_ps1(scalar));
}

FORCEINLINE float __vectorcall vector_get_length(const vector_t v)
{
    return _mm_cvtss_f32(_mm_sqrt_ps(_mm_dp_ps(v, v, 0xff)));
}

FORCEINLINE float __vectorcall vector_get_length_sqaured(const vector_t v)
{
    return _mm_cvtss_f32(_mm_dp_ps(v, v, 0xff));
}

FORCEINLINE vector_t __vectorcall vector_get_norm(const vector_t v)
{
    return _mm_mul_ps(v, _mm_rsqrt_ps(_mm_dp_ps(v, v, 0xff)));
}

FORCEINLINE float __vectorcall vector_get_x(const vector_t v)
{
    return _mm_cvtss_f32(v);
}

FORCEINLINE float __vectorcall vector_get_y(const vector_t v)
{
    const vector_t temp = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
    return _mm_cvtss_f32(temp);
}

FORCEINLINE float __vectorcall vector_get_z(const vector_t v)
{
    const vector_t temp = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
    return _mm_cvtss_f32(temp);
}

FORCEINLINE float __vectorcall vector_get_w(const vector_t v)
{
    const vector_t temp = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
    return _mm_cvtss_f32(temp);
}

FORCEINLINE vector_t __vectorcall vector_set_x(const vector_t v, const float x)
{
    vector_t result = _mm_set_ss(x);
    result = _mm_move_ss(v, result);
    return result;
}

FORCEINLINE vector_t __vectorcall vector_set_y(const vector_t v, const float y)
{
    const vector_t temp = _mm_set_ss(y);
    vector_t result = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 2, 0, 1));
    result = _mm_move_ss(result, temp);
    result = _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 2, 0, 1));
    return result;
}

FORCEINLINE vector_t __vectorcall vector_set_z(const vector_t v, const float z)
{
    const vector_t temp = _mm_set_ss(z);
    vector_t result = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 0, 1, 2));
    result = _mm_move_ss(result, temp);
    result = _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 0, 1, 2));
    return result;
}

FORCEINLINE vector_t __vectorcall vector_set_w(const vector_t v, const float w)
{
    const vector_t temp = _mm_set_ss(w);
    vector_t result = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 2, 1, 3));
    result = _mm_shuffle_ps(result, result, _MM_SHUFFLE(0, 2, 1, 3));
    result = _mm_move_ss(result, temp);
    return result;
}

FORCEINLINE float __vectorcall vector_cross2(const vector_t v0, const vector_t v1)
{
    // v0.x * v1.y - v1.x * v0.y

    vector_t result = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(0, 1, 0, 1));
    result = _mm_mul_ps(v0, result);
    const vector_t temp = _mm_shuffle_ps(result, result, _MM_SHUFFLE(1, 1, 1, 1));
    result = _mm_sub_ps(result, temp);
    return vector_get_x(result);
    //return result;
}

FORCEINLINE vector_t __vectorcall vector2_to_vector(const vector2_t* p_v)
{
    return vector_set(p_v->x, p_v->y, 1.0f, 0.0f);
}

FORCEINLINE vector_t __vectorcall vector3_to_vector(const vector3_t* p_v)
{
    return vector_set(p_v->x, p_v->y, p_v->z, 1.0f);
}

FORCEINLINE vector_t __vectorcall vector4_to_vector(const vector4_t* p_v)
{
    return vector_set(p_v->x, p_v->y, p_v->z, p_v->z);
}

FORCEINLINE vector2_t __vectorcall vector_to_vector2(const vector_t v)
{
    const vector2_t result =
    {
        vector_get_x(v),
        vector_get_y(v)
    };
    return result;
}

FORCEINLINE vector3_t __vectorcall vector_to_vector3(const vector_t v)
{
    const vector3_t result =
    {
        vector_get_x(v),
        vector_get_y(v),
        vector_get_z(v)
    };
    return result;
}

FORCEINLINE vector4_t __vectorcall vector_to_vector4(const vector_t v)
{
    const vector4_t result =
    {
        vector_get_x(v),
        vector_get_y(v),
        vector_get_z(v),
        vector_get_w(v)
    };
    return result;
}

END_EXTERN_C

#endif // VECTOR_H