//***************************************************************************
// 
// 파일: file_system.c
// 
// 설명: 파일 시스템
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/10/15
// 
//***************************************************************************

#include "precompiled.h"

#include "i_file_system.h"

extern bool create_texture2_private(i_texture2_t** pp_out_texture2);
extern bool initialize_texture2_private(i_texture2_t* p_this, const size_t width, const size_t height, const char* p_bitmap);

typedef struct file_system
{
    i_file_system_t base;
    size_t ref_count;

    i_map_t* p_registered_file;     // { uint32_t, i_texture2_t }
} file_system_t;

void __stdcall create_instance(void** pp_out_instance);

static size_t __stdcall add_ref(i_file_system_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    file_system_t* p_file_system = (file_system_t*)p_this;
    return ++p_file_system->ref_count;
}

static size_t __stdcall release(i_file_system_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    file_system_t* p_file_system = (file_system_t*)p_this;
    if (--p_file_system->ref_count == 0)
    {
        destroy_map(p_file_system->p_registered_file);

        SAFE_FREE(p_file_system);

        return 0;
    }

    return p_file_system->ref_count;
}

static size_t __stdcall get_ref_count(const i_file_system_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const file_system_t* p_file_system = (file_system_t*)p_this;
    return p_file_system->ref_count;
}

static bool __stdcall initialize(i_file_system_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    file_system_t* p_file_system = (file_system_t*)p_this;

    create_map(&p_file_system->p_registered_file);
    if (!p_file_system->p_registered_file->vtbl->initialize(p_file_system->p_registered_file, sizeof(uint32_t), sizeof(i_texture2_t), 100))
    {
        ASSERT(false, "Failed to init registered file");
        goto failed_init_registered_file;
    }

    return true;

failed_init_registered_file:
    return false;
}

static bool __stdcall load_a8r8g8b8_dds(const i_file_system_t* p_this, const char* filename, i_texture2_t** pp_out_texture2)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(filename != NULL, "filename == NULL");
    ASSERT(pp_out_texture2 != NULL, "pp_out_texture2 == NULL");

    const file_system_t* p_file_system = (file_system_t*)p_this;

    const uint32_t hash = hash32_fnv1a(filename, strlen(filename));

    i_texture2_t** pp_texture = (i_texture2_t**)p_file_system->p_registered_file->vtbl->get_value_or_null(p_file_system->p_registered_file, &hash, sizeof(uint32_t));
    if (pp_texture != NULL)
    {
        (*pp_texture)->vtbl->add_ref(*pp_texture);
        *pp_out_texture2 = *pp_texture;

        return true;
    }

    i_texture2_t* p_texture;
    create_texture2_private(&p_texture);
    if (p_texture == NULL)
    {
        ASSERT(false, "Failed to create texture2");
        goto failed_create_texture;
    }
    p_file_system->p_registered_file->vtbl->insert(p_file_system->p_registered_file, &hash, sizeof(uint32_t), p_texture, sizeof(i_texture2_t*));

    FILE* p_file = fopen(filename, "rb");
    if (p_file == NULL)
    {
        ASSERT(false, "Failed to open file");
        goto failed_open_file;
    }

    uint32_t magic;
    fread(&magic, sizeof(uint32_t), 1, p_file);

    const char* p_magic = (const char*)&magic;
    if (p_magic[0] != 'D'
        || p_magic[1] != 'D'
        || p_magic[2] != 'S'
        || p_magic[3] != ' ')
    {
        ASSERT(false, "mismatch header");
        goto mismatch_header;
    }

    char dds_header[124];
    fread(dds_header, sizeof(char), 124, p_file);

    const size_t height = *(uint32_t*)&dds_header[8];
    const size_t width = *(uint32_t*)&dds_header[12];

    char* pa_bitmap = (char*)malloc(width * height * sizeof(uint32_t));
    if (pa_bitmap == NULL)
    {
        ASSERT(false, "Failed to malloc bitmap");
        goto failed_malloc_bitmap;
    }

    fread(pa_bitmap, 1, width * height * sizeof(uint32_t), p_file);
    
    if (!initialize_texture2_private(p_texture, width, height, pa_bitmap))
    {
        ASSERT(false, "Failed to init texture");
        goto failed_init_texture;
    }

    *pp_out_texture2 = p_texture;

    fclose(p_file);

    return true;

failed_init_texture:
failed_malloc_bitmap:
mismatch_header:
    fclose(p_file);

failed_open_file:
    p_texture->vtbl->release(p_texture);

failed_create_texture:
    *pp_out_texture2 = NULL;
    return false;
}

void __stdcall create_instance(void** pp_out_instance)
{
    ASSERT(pp_out_instance != NULL, "pp_out_instance == NULL");

    static i_file_system_vtbl_t vtbl =
    {
        add_ref,
        release,
        get_ref_count,

        initialize,

        load_a8r8g8b8_dds
    };

    file_system_t* pa_file_system = malloc(sizeof(file_system_t));
    if (pa_file_system == NULL)
    {
        ASSERT(false, "Failed to malloc world");
        *pp_out_instance = NULL;
    }

    pa_file_system->base.vtbl = &vtbl;
    pa_file_system->ref_count = 1;

    *pp_out_instance = pa_file_system;
}