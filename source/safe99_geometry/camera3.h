//***************************************************************************
// 
// 파일: camera3.h
// 
// 설명: 3D 카메라
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/11/30
// 
//***************************************************************************

#include "safe99_common/defines.h"
#include "transform3.h"

START_EXTERN_C

FORCEINLINE float __stdcall camera3_get_fov(const camera3_t* p_camera)
{
    ASSERT(p_camera != NULL, "p_camera == NULL");
    return p_camera->fov;
}

FORCEINLINE float __stdcall camera3_get_near_z(const camera3_t* p_camera)
{
    ASSERT(p_camera != NULL, "p_camera == NULL");
    return p_camera->near_z;
}

FORCEINLINE float __stdcall camera3_get_far_z(const camera3_t* p_camera)
{
    ASSERT(p_camera != NULL, "p_camera == NULL");
    return p_camera->far_z;
}

FORCEINLINE view_port_t __stdcall camera3_get_view_port(const camera3_t* p_camera)
{
    ASSERT(p_camera != NULL, "p_camera == NULL");
    return p_camera->view_port;
}

FORCEINLINE void __stdcall camera3_set_fov(camera3_t* p_camera, const float fov)
{
    ASSERT(p_camera != NULL, "p_camera == NULL");
    p_camera->fov = fov;
}

FORCEINLINE void __stdcall camera3_set_near_z(camera3_t* p_camera, const float near_z)
{
    ASSERT(p_camera != NULL, "p_camera == NULL");
    p_camera->near_z = near_z;
}

FORCEINLINE void __stdcall camera3_set_far_z(camera3_t* p_camera, const float far_z)
{
    ASSERT(p_camera != NULL, "p_camera == NULL");
    p_camera->far_z = far_z;
}

FORCEINLINE void __stdcall camera3_set_view_port(camera3_t* p_camera, const view_port_t* p_view_port)
{
    ASSERT(p_camera != NULL, "p_camera == NULL");
    ASSERT(p_view_port != NULL, "p_view_port == NULL");
    p_camera->view_port = *p_view_port;
}

FORCEINLINE void __stdcall camera3_set_look_at(camera3_t* p_camera, const vector3_t at)
{
    ASSERT(p_camera != NULL, "p_camera == NULL");

    const vector_t eye = vector3_to_vector(transform3_get_position(&p_camera->transform));
    const vector_t up = vector_set(0.0f, 1.0f, 0.0f, 0.0f);

    vector_t z = vector_sub(vector3_to_vector(at), eye);
    z = vector_get_unit3(z);

    vector_t x = vector_cross3(up, z);
    x = vector_get_unit3(x);

    vector_t y = vector_cross3(z, x);

    transform3_set_right_vector(&p_camera->transform, vector_to_vector3(x));
    transform3_set_up_vector(&p_camera->transform, vector_to_vector3(y));
    transform3_set_forward_vector(&p_camera->transform, vector_to_vector3(z));
}

FORCEINLINE matrix_t __stdcall camera3_get_view_matrix(const camera3_t* p_camera)
{
    ASSERT(p_camera != NULL, "p_camera == NULL");

    const vector_t right = vector3_to_vector_zero(transform3_get_right_vector(&p_camera->transform));
    const vector_t up = vector3_to_vector_zero(transform3_get_up_vector(&p_camera->transform));
    const vector_t forward = vector3_to_vector_zero(transform3_get_forward_vector(&p_camera->transform));

    const vector_t eye = vector3_to_vector_zero(transform3_get_position(&p_camera->transform));

    matrix_t result;
    result.r0 = right;
    result.r1 = up;
    result.r2 = forward;
    result.r3 = s_identity_r3;
    result = matrix_get_transpose(result);

    result.r3 = vector_set(-vector_dot3(right, eye), -vector_dot3(up, eye), -vector_dot3(forward, eye), 1.0f);
    return result;
}

FORCEINLINE matrix_t __stdcall camera3_get_perspective_matrix(const camera3_t* p_camera)
{
    ASSERT(p_camera != NULL, "p_camera == NULL");

    const float a = p_camera->view_port.width / p_camera->view_port.height;
    const float fov = p_camera->fov;
    const float d = 1.0f / tanf((fov * (float)PI_DIV_180) * 0.5f);
    const float n = p_camera->near_z;
    const float f = p_camera->far_z;

    const matrix_t result = matrix_set(d / a, 0.0f, 0.0f, 0.0f,
                                       0.0f, d, 0.0f, 0.0f,
                                       0.0f, 0.0f, (n + f) / (n - f), 2 * n * f / (n - f),
                                       0.0f, 0.0f, 1.0f, 0.0f);
    return result;
}

END_EXTERN_C