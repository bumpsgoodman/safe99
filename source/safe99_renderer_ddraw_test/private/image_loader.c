#define _CRT_SECURE_NO_WARNINGS

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "safe99_renderer_ddraw_test/image_loader.h"

#define DDS_HEADER_SIZE 124

bool load_a8r8g8b8_dds(const char* filename, image_t* out_image)
{
    if (out_image == NULL) {
        return false;
    }

    FILE* p_file = fopen(filename, "rb");
    if (p_file == NULL) {
        return false;
    }

    uint32_t magic;
    fread(&magic, sizeof(uint32_t), 1, p_file);

    const char* p_magic = (const char*)&magic;
    if (p_magic[0] != 'D'
        || p_magic[1] != 'D'
        || p_magic[2] != 'S'
        || p_magic[3] != ' ') {
        fclose(p_file);
        return false;
    }

    char dds_header[DDS_HEADER_SIZE];
    fread(dds_header, sizeof(char), DDS_HEADER_SIZE, p_file);

    const uint32_t height = *(uint32_t*)&dds_header[8];
    const uint32_t width = *(uint32_t*)&dds_header[12];

    char* pa_bitmap = (char*)malloc(width * height * sizeof(uint32_t));
    if (pa_bitmap == NULL) {
        fclose(p_file);
        return false;
    }

    fread(pa_bitmap, 1, width * height * sizeof(uint32_t), p_file);

    out_image->width = width;
    out_image->height = height;
    out_image->pa_bitmap = pa_bitmap;

    fclose(p_file);
    return true;
}