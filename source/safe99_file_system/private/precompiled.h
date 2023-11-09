//***************************************************************************
// 
// 파일: precompiled.h
// 
// 설명: precompiled 헤더
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/10/15
// 
//***************************************************************************

#ifndef PRECOMPILED_H
#define PRECOMPILED_H

#define _CRT_SECURE_NO_WARNINGS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <string.h>
#include <Windows.h>

#include "safe99_common/defines.h"

#include "safe99_generic/map.h"
#include "safe99_generic/util/hash_function.h"

#endif // PRECOMPILED_H