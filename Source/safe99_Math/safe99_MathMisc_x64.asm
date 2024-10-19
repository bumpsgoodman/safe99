; /*
;  * Math misc x64 어셈
; 작성자: bumpsgoodman
; 작성일: 2024-08-24
; */

.code

; int Log2Int64(const uint64_t num);
; num = rcx
Log2Int64 PROC EXPORT
    bsr rax, rcx
    ret
Log2Int64 ENDP

END