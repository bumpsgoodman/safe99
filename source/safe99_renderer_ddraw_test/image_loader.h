#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <stdbool.h>

#include "image.h"
#include "safe99_common/defines.h"

START_EXTERN_C

bool load_a8r8g8b8_dds(const char* filename, image_t* out_image);

END_EXTERN_C

#endif // IMAGE_LOADER_H