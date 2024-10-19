// 작성자: bumpsgoodman
// 작성일: 2024-08-10

#include "Precompiled.h"
#include "ErrorCode.h"

static safe99_error_code_t s_errorCode = SAFE99_ERROR_CODE_SUCCESS;

safe99_error_code_t __stdcall safe99_GetLastError()
{
    return s_errorCode;
}

void __stdcall safe99_SetLastError(const safe99_error_code_t errorCode)
{
    s_errorCode = errorCode;
}