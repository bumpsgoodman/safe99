// 작성자: bumpsgoodman
// 작성일: 2024-08-10

#ifndef SAFE99_ERROR_CODE_H
#define SAFE99_ERROR_CODE_H

typedef unsigned int safe99_error_code_t;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

safe99_error_code_t     __stdcall   safe99_GetLastError();
void                    __stdcall   safe99_SetLastError(const safe99_error_code_t errorCode);

#ifdef __cplusplus
}
#endif // __cplusplus

#define SAFE99_ERROR_CODE_SUCCESS   1
#define SAFE99_ERROR_CODE_FAIL      0

#define SAFE99_IS_FAILED(errorCode)     ((errorCode) == SAFE99_ERROR_CODE_FAIL)

#define SAFE99_ERROR_CODE_MAJOR         (1 << 24)

#define SAFE99_ERROR_CODE_MEM_POOL                                  (SAFE99_ERROR_CODE_MAJOR | 0)
#define SAFE99_ERROR_CODE_MEM_POOL_ADD_BLOCK                        (SAFE99_ERROR_CODE_MEM_POOL | 1)
#define SAFE99_ERROR_CODE_MEM_POOL_ALLOC_FULL                       (SAFE99_ERROR_CODE_MEM_POOL | 2)
#define SAFE99_ERROR_CODE_MEM_POOL_FREE_INVALID_MEM                 (SAFE99_ERROR_CODE_MEM_POOL | 3)

#define SAFE99_ERROR_CODE_CONTAINER                                 (SAFE99_ERROR_CODE_MAJOR + 1)
#define SAFE99_ERROR_CODE_CONTAINER_FULL                            (SAFE99_ERROR_CODE_CONTAINER | 1)
#define SAFE99_ERROR_CODE_CONTAINER_EMPTY                           (SAFE99_ERROR_CODE_CONTAINER | 2)
#define SAFE99_ERROR_CODE_CONTAINER_INVALID_INDEX                   (SAFE99_ERROR_CODE_CONTAINER | 3)

#define SAFE99_ERROR_CODE_RENDERER                                  (SAFE99_ERROR_CODE_MAJOR + 5)
#define SAFE99_ERROR_CODE_RENDERER_CREATE_DEVICE                    (SAFE99_ERROR_CODE_RENDERER | 1)
#define SAFE99_ERROR_CODE_RENDERER_CREATE_SWAP_CHAIN                (SAFE99_ERROR_CODE_RENDERER | 2)
#define SAFE99_ERROR_CODE_RENDERER_CREATE_RENDER_TARGET_VIEW        (SAFE99_ERROR_CODE_RENDERER | 2)
#define SAFE99_ERROR_CODE_RENDERER_FAILED_GET_BUFFER                (SAFE99_ERROR_CODE_RENDERER | 2)

#endif // SAFE99_ERROR_CODE_H