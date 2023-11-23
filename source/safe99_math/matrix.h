//***************************************************************************
// 
// 파일: matrix.h
// 
// 설명: SSE로 작성한 4x4 행렬
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/09/27
// 
//***************************************************************************

#ifndef MATRIX_H
#define MATRIX_H

#include "math_misc.h"
#include "vector.h"

static const vector_t s_identity_r0 = { 1.0f, 0.0f, 0.0f, 0.0f };
static const vector_t s_identity_r1 = { 0.0f, 1.0f, 0.0f, 0.0f };
static const vector_t s_identity_r2 = { 0.0f, 0.0f, 1.0f, 0.0f };
static const vector_t s_identity_r3 = { 0.0f, 0.0f, 0.0f, 1.0f };

START_EXTERN_C

FORCEINLINE matrix_t __vectorcall matrix_get_identity(void)
{
    matrix_t result;
    result.r0 = s_identity_r0;
    result.r1 = s_identity_r1;
    result.r2 = s_identity_r2;
    result.r3 = s_identity_r3;
    return result;
}

FORCEINLINE matrix_t __vectorcall matrix_get_transpose(matrix_t m)
{
    _MM_TRANSPOSE4_PS(m.r0, m.r1, m.r2, m.r3);
    return m;
}

FORCEINLINE matrix_t __vectorcall matrix_set(const float m00, const float m01, const float m02, const float m03,
                                             const float m10, const float m11, const float m12, const float m13,
                                             const float m20, const float m21, const float m22, const float m23,
                                             const float m30, const float m31, const float m32, const float m33)
{
    matrix_t result;
    result.r0 = vector_set(m00, m01, m02, m03);
    result.r1 = vector_set(m10, m11, m12, m13);
    result.r2 = vector_set(m20, m21, m22, m23);
    result.r3 = vector_set(m30, m31, m32, m33);
    return result;
}

FORCEINLINE vector_t __vectorcall matrix_mul_vector(const vector_t v, const matrix_t m_tr)
{
    const vector_t result = vector_set(_mm_cvtss_f32(_mm_dp_ps(v, m_tr.r0, 0xff)),
                                       _mm_cvtss_f32(_mm_dp_ps(v, m_tr.r1, 0xff)),
                                       _mm_cvtss_f32(_mm_dp_ps(v, m_tr.r2, 0xff)),
                                       _mm_cvtss_f32(_mm_dp_ps(v, m_tr.r3, 0xff)));
    return result;
}

FORCEINLINE matrix_t __vectorcall matrix_mul(const matrix_t m0, const matrix_t m1_tr)
{
    matrix_t result;
    result.r0 = vector_set(_mm_cvtss_f32(_mm_dp_ps(m0.r0, m1_tr.r0, 0xff)),
                           _mm_cvtss_f32(_mm_dp_ps(m0.r0, m1_tr.r1, 0xff)),
                           _mm_cvtss_f32(_mm_dp_ps(m0.r0, m1_tr.r2, 0xff)),
                           _mm_cvtss_f32(_mm_dp_ps(m0.r0, m1_tr.r3, 0xff)));

    result.r1 = vector_set(_mm_cvtss_f32(_mm_dp_ps(m0.r1, m1_tr.r0, 0xff)),
                           _mm_cvtss_f32(_mm_dp_ps(m0.r1, m1_tr.r1, 0xff)),
                           _mm_cvtss_f32(_mm_dp_ps(m0.r1, m1_tr.r2, 0xff)),
                           _mm_cvtss_f32(_mm_dp_ps(m0.r1, m1_tr.r3, 0xff)));

    result.r2 = vector_set(_mm_cvtss_f32(_mm_dp_ps(m0.r2, m1_tr.r0, 0xff)),
                           _mm_cvtss_f32(_mm_dp_ps(m0.r2, m1_tr.r1, 0xff)),
                           _mm_cvtss_f32(_mm_dp_ps(m0.r2, m1_tr.r2, 0xff)),
                           _mm_cvtss_f32(_mm_dp_ps(m0.r2, m1_tr.r3, 0xff)));

    result.r3 = vector_set(_mm_cvtss_f32(_mm_dp_ps(m0.r3, m1_tr.r0, 0xff)),
                           _mm_cvtss_f32(_mm_dp_ps(m0.r3, m1_tr.r1, 0xff)),
                           _mm_cvtss_f32(_mm_dp_ps(m0.r3, m1_tr.r2, 0xff)),
                           _mm_cvtss_f32(_mm_dp_ps(m0.r3, m1_tr.r3, 0xff)));
    return result;
}

FORCEINLINE matrix_t __vectorcall matrix_get_scale_2d(const float x, const float y)
{
    matrix_t result;
    result.r0 = vector_set(x, 0.0f, 0.0f, 0.0f);
    result.r1 = vector_set(0.0f, y, 0.0f, 0.0f);
    result.r2 = s_identity_r2;
    result.r3 = s_identity_r3;
    return result;
}

FORCEINLINE matrix_t __vectorcall matrix_get_rotation_2d(const float rad)
{
    matrix_t result;
    float sin;
    float cos;

    get_sin_cos(rad, &sin, &cos);
    result.r0 = vector_set(cos, -sin, 0.0f, 0.0f);
    result.r1 = vector_set(sin, cos, 0.0f, 0.0f);
    result.r2 = s_identity_r2;
    result.r3 = s_identity_r3;
    return result;
}

FORCEINLINE matrix_t __vectorcall matrix_get_translation_2d(const float x, const float y)
{
    matrix_t result;
    result.r0 = vector_set(1.0f, 0.0f, x, 0);
    result.r1 = vector_set(0.0f, 1.0f, y, 0);
    result.r2 = s_identity_r2;
    result.r3 = s_identity_r3;
    return result;
}

FORCEINLINE matrix_t __vectorcall matrix_get_scale_3d(const float x, const float y, const float z)
{
    matrix_t result;
    result.r0 = vector_set(x, 0.0f, 0.0f, 0.0f);
    result.r1 = vector_set(0.0f, y, 0.0f, 0.0f);
    result.r2 = vector_set(0.0f, 0.0f, z, 0.0f);
    result.r3 = s_identity_r3;
    return result;
}

FORCEINLINE matrix_t __vectorcall matrix_get_translation_3d(const float x, const float y, const float z)
{
    matrix_t result;
    result.r0 = vector_set(1.0f, 0.0f, 0.0f, x);
    result.r1 = vector_set(0.0f, 1.0f, 0.0f, y);
    result.r2 = vector_set(0.0f, 0.0f, 1.0f, z);
    result.r3 = s_identity_r3;
    return result;
}

FORCEINLINE matrix_t __vectorcall matrix_get_rotation_3d(const float yaw, const float pitch, const float roll)
{
    float roll_sin;
    float roll_cos;
    float pitch_sin;
    float pitch_cos;
    float yaw_sin;
    float yaw_cos;
    get_sin_cos(roll, &roll_sin, &roll_cos);
    get_sin_cos(pitch, &pitch_sin, &pitch_cos);
    get_sin_cos(yaw, &yaw_sin, &yaw_cos);

    matrix_t yaw_mat;
    yaw_mat.r0 = s_identity_r0;
    yaw_mat.r1 = vector_set(0.0f, yaw_cos, -yaw_sin, 0.0f);
    yaw_mat.r2 = vector_set(0.0f, yaw_sin, yaw_cos, 0.0f);
    yaw_mat.r3 = s_identity_r3;

    matrix_t pitch_mat;
    pitch_mat.r0 = vector_set(pitch_cos, pitch_sin, 0.0f, 0.0f);
    pitch_mat.r1 = s_identity_r1;
    pitch_mat.r2 = vector_set(-pitch_sin, pitch_cos, 1.0f, 0.0f);
    pitch_mat.r3 = s_identity_r3;

    matrix_t roll_mat;
    roll_mat.r0 = vector_set(roll_cos, -roll_sin, 0.0f, 0.0f);
    roll_mat.r1 = vector_set(roll_sin, roll_cos, 0.0f, 0.0f);
    roll_mat.r2 = s_identity_r2;
    roll_mat.r3 = s_identity_r3;

    matrix_t result = matrix_mul(yaw_mat, matrix_get_transpose(pitch_mat));
    result = matrix_mul(result, matrix_get_transpose(roll_mat));
    return result;

#if 0
    matrix_t result;
    result.r0 = vector_set(yaw_cos * pitch_cos + yaw_sin * pitch_sin * roll_sin, -yaw_cos * pitch_sin + yaw_sin * pitch_sin * roll_cos, yaw_sin * pitch_cos, 0.0f);
    result.r1 = vector_set(pitch_cos * roll_sin, pitch_cos * roll_cos, -pitch_sin, 0.0f);
    result.r2 = vector_set(-yaw_sin * pitch_cos + yaw_cos * pitch_sin * roll_sin, yaw_sin * pitch_sin + yaw_cos * pitch_sin * roll_cos, yaw_cos * pitch_cos, 0.0f);
    result.r3 = s_identity_r3;
    return result;
#endif
}

END_EXTERN_C

#endif // MATRIX_H