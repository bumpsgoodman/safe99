//***************************************************************************
// 
// 파일: math_misc_x86.h
// 
// 설명: 기타 수학 함수 (x86)
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/11/20
// 
//***************************************************************************

#include "safe99_common/defines.h"
#include "safe99_math/math_misc.h"

__declspec(naked) int __stdcall log2int64(const uint64_t num)
{
    // sp로 매개변수에 접근하기 때문에 함수 프롤로그/에필로그 작성 x

    __asm
    {
        ; 상위 32비트 불러오기
        mov eax, dword ptr[esp + 8]

        ; 상위 32비트가 0인지 판단
        test eax, eax
        je zero

        ; 하위 32비트에 대해서 bsr 연산 후, 32 더해줌
        bsr eax, eax
        add eax, 32
        ret 8

    zero:
        bsr eax, dword ptr[esp + 4]
        ret 8
    }
}