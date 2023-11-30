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

FORCEINLINE void __stdcall camera3_set_look_at(camera3_t* p_camera, const vector3_t at)
{
    ASSERT(p_camera != NULL, "p_camera == NULL");

    const vector_t eye = vector3_to_vector(transform3_get_position(&p_camera->transform));
    const vector_t up = vector_set(0.0f, 1.0f, 0.0f, 0.0f);

    vector_t temp_at = vector3_to_vector(at);

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

    matrix_t view_matrix;
    view_matrix.r0 = right;
    view_matrix.r1 = up;
    view_matrix.r2 = forward;
    view_matrix.r3 = vector_set(-vector_dot3(eye, right), -vector_dot3(eye, up), -vector_dot3(eye, forward), 1.0f);

    return view_matrix;
}

END_EXTERN_C