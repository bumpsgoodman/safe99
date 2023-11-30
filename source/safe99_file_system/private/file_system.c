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

typedef struct vector3_packed
{
    float x;
    float y;
    float z;
} vector3_packed_t;

typedef struct vector3_int_packed
{
    int x;
    int y;
    int z;
} vector3_int_packed_t;

extern bool create_texture_private(i_texture_t** pp_out_texture);
extern bool initialize_texture_private(i_texture_t* p_this, const size_t width, const size_t height, const char* p_bitmap);

extern bool create_safe3d_private(i_safe3d_t** pp_out_safe3d);
extern bool initialize_safe3d_private(i_safe3d_t* p_this,
                                      vector3_t* p_vertices, const size_t num_vertices,
                                      uint_t* p_indices, const size_t num_indices,
                                      const color_t wireframe_color);

typedef struct file_system
{
    i_file_system_t base;
    size_t ref_count;

    map_t registered_files;     // { uint64_t : void* }
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
        map_release(&p_file_system->registered_files);

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

    if (!map_initialize(&p_file_system->registered_files, sizeof(uint64_t), sizeof(i_texture_t), 100))
    {
        ASSERT(false, "Failed to init registered file");
        goto failed_init_registered_file;
    }

    return true;

failed_init_registered_file:
    memset(p_file_system, 0, sizeof(file_system_t));
    return false;
}

static bool __stdcall load_a8r8g8b8_dds(i_file_system_t* p_this, const char* filename, i_texture_t** pp_out_texture2)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(filename != NULL, "filename == NULL");
    ASSERT(pp_out_texture2 != NULL, "pp_out_texture2 == NULL");

    file_system_t* p_file_system = (file_system_t*)p_this;

    // 이미 해당 텍스쳐가 있으면 등록된 텍스쳐 반환
    const uint64_t hash = hash32_fnv1a(filename, strlen(filename));
    i_texture_t** pp_texture = (i_texture_t**)map_get_value_by_hash_or_null(&p_file_system->registered_files,
                                                                              hash, &hash, sizeof(uint64_t));
    if (pp_texture != NULL)
    {
        (*pp_texture)->vtbl->add_ref(*pp_texture);
        *pp_out_texture2 = *pp_texture;

        return true;
    }

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

    fclose(p_file);

    i_texture_t* p_texture;
    create_texture_private(&p_texture);
    if (!initialize_texture_private(p_texture, width, height, pa_bitmap))
    {
        ASSERT(false, "Failed to init texture");
        p_texture->vtbl->release(p_texture);
        return false;
    }

    map_insert_by_hash(&p_file_system->registered_files, hash, &hash, sizeof(uint64_t), p_texture, sizeof(i_texture_t*));

    *pp_out_texture2 = p_texture;

    return true;

failed_malloc_bitmap:
mismatch_header:
    fclose(p_file);

failed_open_file:
    *pp_out_texture2 = NULL;
    return false;
}

bool __stdcall load_safe3d(i_file_system_t* p_this, const char* filename, i_safe3d_t** pp_out_safe3d)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(filename != NULL, "filename == NULL");
    ASSERT(pp_out_safe3d != NULL, "pp_out_safe3d == NULL");

    file_system_t* p_file_system = (file_system_t*)p_this;

    // 이미 해당 텍스쳐가 있으면 등록된 텍스쳐 반환
    const uint64_t hash = hash32_fnv1a(filename, strlen(filename));
    i_safe3d_t** pp_safe3d = (i_safe3d_t**)map_get_value_by_hash_or_null(&p_file_system->registered_files,
                                                                         hash, &hash, sizeof(uint64_t));
    if (pp_safe3d != NULL)
    {
        (*pp_safe3d)->vtbl->add_ref(*pp_safe3d);
        *pp_out_safe3d = *pp_safe3d;

        return true;
    }

    FILE* p_file = fopen(filename, "rb");
    if (p_file == NULL)
    {
        ASSERT(false, "Failed to open file");
        goto failed_open_file;
    }

    char magic[8] = { 0, };
    fread(&magic, sizeof(magic), 1, p_file);

    if (strcmp(magic, "safe3d") != 0)
    {
        ASSERT(false, "mismatch header");
        goto mismatch_header;
    }

    uint_t num_vertices;
    fread(&num_vertices, sizeof(uint_t), 1, p_file);

    uint_t num_indices;
    fread(&num_indices, sizeof(uint_t), 1, p_file);

    color_t wireframe_color;
    fread(&wireframe_color, sizeof(color_t), 1, p_file);

    // 버텍스
    vector3_t* pa_vertices = (vector3_t*)malloc(sizeof(vector3_t) * num_vertices);
    if (pa_vertices == NULL)
    {
        ASSERT(false, "Failed to malloc vertices");
        goto failed_malloc_vertices;
    }

    for (size_t i = 0; i < (size_t)num_vertices; ++i)
    {
        vector3_packed_t v;
        fread(&v, sizeof(vector3_packed_t), 1, p_file);

        pa_vertices[i].x = v.x;
        pa_vertices[i].y = v.y;
        pa_vertices[i].z = v.z;
    }

    // 인덱스
    uint_t* pa_indices = (uint_t*)malloc(sizeof(uint_t) * num_indices * 3);
    if (pa_indices == NULL)
    {
        ASSERT(false, "Failed to malloc indices");
        goto failed_malloc_indices;
    }

    for (size_t i = 0; i < (size_t)num_indices; ++i)
    {
        vector3_int_packed_t v;
        fread(&v, sizeof(vector3_int_packed_t), 1, p_file);

        size_t index = i * 3;
        pa_indices[index] = v.x;
        pa_indices[index + 1] = v.y;
        pa_indices[index + 2] = v.z;
    }

    fclose(p_file);

    i_safe3d_t* p_safe3d;
    create_safe3d_private(&p_safe3d);
    if (!initialize_safe3d_private(p_safe3d, pa_vertices, num_vertices, pa_indices, num_indices * 3, wireframe_color))
    {
        ASSERT(false, "Failed to init texture");
        SAFE_FREE(p_safe3d);
        return false;
    }

    map_insert_by_hash(&p_file_system->registered_files, hash, &hash, sizeof(uint64_t), p_safe3d, sizeof(i_safe3d_t*));

    *pp_out_safe3d = p_safe3d;

    return true;

failed_malloc_indices:
    SAFE_FREE(pa_vertices);

failed_malloc_vertices:
mismatch_header:
    fclose(p_file);

failed_open_file:
    *pp_out_safe3d = NULL;
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

        load_a8r8g8b8_dds,
        load_safe3d
    };

    file_system_t* pa_file_system = malloc(sizeof(file_system_t));
    if (pa_file_system == NULL)
    {
        ASSERT(false, "Failed to malloc file system");
        *pp_out_instance = NULL;
    }

    pa_file_system->base.vtbl = &vtbl;
    pa_file_system->ref_count = 1;

    *pp_out_instance = pa_file_system;
}