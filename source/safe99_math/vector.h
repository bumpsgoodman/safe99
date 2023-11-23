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

#include "safe99_common/defines.h"

#define TO_VECTOR(v) (*(vector_t*)(&v))
#define TO_VECTOR2(v) (*(vector2_t*)(&v))
#define TO_VECTOR3(v) (*(vector3_t*)(&v))
#define TO_VECTOR4(v) (*(vector4_t*)(&v))

#define CAST_TO_VECTOR ((vector_t*)(&v))

START_EXTERN_C

FORCEINLINE vector_t __vectorcall vector_set(const float x, const float y, const float z, const float w)
{
    return _mm_set_ps(w, z, y, x);
}

FORCEINLINE vector_t __vectorcall vector_set1(const float x)
{
    return _mm_set_ps1(x);
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
    result = _mm_move_ss(result, temp);
    result = _mm_shuffle_ps(result, result, _MM_SHUFFLE(0, 2, 1, 3));
    return result;
}

FORCEINLINE vector_t __vectorcall vector_get_zero(void)
{
    return _mm_setzero_ps();
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

FORCEINLINE vector_t __vectorcall vector2_to_vector(const vector2_t* p_v)
{
    ASSERT(p_v != NULL, "p_v == NULL");
    return vector_set(p_v->x, p_v->y, 1.0f, 1.0f);
}

FORCEINLINE vector_t __vectorcall vector3_to_vector(const vector3_t* p_v)
{
    ASSERT(p_v != NULL, "p_v == NULL");
    return vector_set(p_v->x, p_v->y, p_v->z, 1.0f);
}

FORCEINLINE vector_t __vectorcall vector4_to_vector(const vector4_t* p_v)
{
    ASSERT(p_v != NULL, "p_v == NULL");
    return vector_set(p_v->x, p_v->y, p_v->z, p_v->w);
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

FORCEINLINE vector_t __vectorcall vector_div(const vector_t v0, const vector_t v1)
{
    return _mm_div_ps(v0, v1);
}

FORCEINLINE vector_t __vectorcall vector_max(const vector_t v0, const vector_t v1)
{
    return _mm_max_ps(v0, v1);
}

FORCEINLINE vector_t __vectorcall vector_min(const vector_t v0, const vector_t v1)
{
    return _mm_min_ps(v0, v1);
}

FORCEINLINE vector_t __vectorcall vector_round(const vector_t v)
{
    return _mm_round_ps(v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}

FORCEINLINE vector_t __vectorcall vector_floor(const vector_t v)
{
    return _mm_round_ps(v, _MM_FROUND_FLOOR);
}

FORCEINLINE vector_t __vectorcall vector_ceil(const vector_t v)
{
    return _mm_round_ps(v, _MM_FROUND_CEIL);
}

FORCEINLINE vector_t __vectorcall vector_trunc(const vector_t v)
{
    return _mm_trunc_ps(v);
}

FORCEINLINE vector_t __vectorcall vector_mod(const vector_t v0, const vector_t v1)
{
    // v0 % v1 = v0 - v1 * TRUNC(v0 / v1);
    vector_t result = vector_trunc(vector_div(v0, v1));
    result = vector_sub(v0, vector_mul(v1, result));
    return result;
}

FORCEINLINE float __vectorcall vector_get_length2(const vector_t v)
{
    return _mm_cvtss_f32(_mm_sqrt_ps(_mm_dp_ps(v, v, 0x3f)));
}

FORCEINLINE float __vectorcall vector_get_length3(const vector_t v)
{
    return _mm_cvtss_f32(_mm_sqrt_ps(_mm_dp_ps(v, v, 0x7f)));
}

FORCEINLINE float __vectorcall vector_get_length2_sqaured(const vector_t v)
{
    return _mm_cvtss_f32(_mm_dp_ps(v, v, 0x3f));
}

FORCEINLINE float __vectorcall vector_get_length3_sqaured(const vector_t v)
{
    return _mm_cvtss_f32(_mm_dp_ps(v, v, 0x7f));
}

FORCEINLINE vector_t __vectorcall vector_get_norm2(const vector_t v)
{
    return _mm_mul_ps(v, _mm_rsqrt_ps(_mm_dp_ps(v, v, 0x3f)));
}

FORCEINLINE vector_t __vectorcall vector_get_norm3(const vector_t v)
{
    return _mm_mul_ps(v, _mm_rsqrt_ps(_mm_dp_ps(v, v, 0x7f)));
}

FORCEINLINE float __vectorcall vector_cross2(const vector_t v0, const vector_t v1)
{
    // v0.x * v1.y - v1.x * v0.y

    vector_t result = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(0, 1, 0, 1));
    result = _mm_mul_ps(v0, result);
    const vector_t temp = _mm_shuffle_ps(result, result, _MM_SHUFFLE(1, 1, 1, 1));
    result = _mm_sub_ps(result, temp);
    return vector_get_x(result);
}

FORCEINLINE float __vectorcall vector_dot2(const vector_t v0, const vector_t v1)
{
    return _mm_cvtss_f32(_mm_dp_ps(v0, v1, 0x3f));
}

FORCEINLINE float __vectorcall vector_dot3(const vector_t v0, const vector_t v1)
{
    return _mm_cvtss_f32(_mm_dp_ps(v0, v1, 0x7f));
}

FORCEINLINE vector_t __vectorcall vector_clamp(const vector_t v, const vector_t min_v, const vector_t max_v)
{
    vector_t result = vector_min(v, max_v);
    return vector_max(result, min_v);
}

FORCEINLINE vector_t __vectorcall vector_clamp_scalar(const vector_t v, const float min, const float max)
{
    return vector_clamp(v, vector_set1(min), vector_set1(max));
}

FORCEINLINE vector_t __vectorcall vector_wrap(const vector_t v, const vector_t min_v, const vector_t max_v)
{
    /*
    if (v > max_v)
    {
        return min_v + mod(v - max_v, max_v);
    }
    else if (v < min_v)
    {
        return max_v + mod(min_v + v, max_v);
    }
    */

    // v가 최대 값보다 클 때
    const vector_t max_mask = _mm_cmpgt_ps(v, max_v);
    vector_t max_result = _mm_and_ps(v, max_mask);
    max_result = vector_sub(max_result, max_v);
    max_result = vector_mod(max_result, max_v);
    max_result = vector_add(min_v, max_result);
    max_result = _mm_and_ps(max_result, max_mask);

    // v가 최소 값보다 작을 때
    const vector_t min_mask = _mm_cmplt_ps(v, min_v);
    vector_t min_result = _mm_and_ps(v, min_mask);
    min_result = vector_add(min_result, min_v);
    min_result = vector_mod(min_result, max_v);
    min_result = vector_add(max_v, min_result);
    min_result = _mm_and_ps(min_result, min_mask);

    // and를 하고 나면 순서가 바뀌기 때문에 역순으로 바꾸기
    vector_t result = _mm_or_ps(min_result, max_result);
    result = _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 2, 1, 0));
    return result;
}

FORCEINLINE vector_t __vectorcall vector_wrap_scalar(const vector_t v, const float min, const float max)
{
    return vector_wrap(v, vector_set1(min), vector_set1(max));
}

END_EXTERN_C

#endif // VECTOR_H