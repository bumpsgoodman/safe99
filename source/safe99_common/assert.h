//***************************************************************************
// 
// 파일: assert.h
// 
// 설명: assert 매크로 함수
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/07/02
// 
//***************************************************************************

#ifndef ASSERT_H
#define ASSERT_H

#if defined(NDEBUG)
#define ASSERT(cond, msg) ((void)0)
#else
#include <intrin.h>
#define ASSERT(cond, msg) { if (!(cond)) { __debugbreak(); } }
#endif // NDBUG

#endif // ASSERT_H