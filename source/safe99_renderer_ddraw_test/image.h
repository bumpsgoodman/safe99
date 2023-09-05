#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

typedef struct image {
    uint32_t width;
    uint32_t height;
    char* pa_bitmap;
} image_t;

#endif // IMAGE_H