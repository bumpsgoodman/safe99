//***************************************************************************
// 
// 파일: safe_delete.h
// 
// 설명: safe delete 매크로 함수
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/07/02
// 
//***************************************************************************

#ifndef SAFE_DELETE_H
#define SAFE_DELETE_H

#include <stdlib.h>

#define SAFE_FREE(p)         { free((p)); (p) = NULL; }
#define SAFE_RELEASE(p)      { if ((p)) { (p)->Release(); (p) = NULL; } }

#endif // SAFE_DELETE_H