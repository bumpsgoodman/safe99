// 작성자: bumpsgoodman
// 작성일: 2024-08-10

#ifndef SAFE99_PRIMITIVE_TYPE_H
#define SAFE99_PRIMITIVE_TYPE_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef unsigned int uint_t;

#ifndef PTR_SIZE
    #define PTR_SIZE sizeof(void*)
#endif // PTR_SIZE

#endif SAFE99_PRIMITIVE_TYPE_H