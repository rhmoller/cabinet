#ifndef CABINET_H
#define CABINET_H

typedef enum {
    CAB_FLOAT,
} ElementType;

typedef enum {
    CAB_TRIANGLES,
} DrawMode;

typedef struct AttributeCfg {
    char *name;
    int buffer;
    int size;
    ElementType type;
    int stride;
    int offset;
} AttributeCfg;

typedef struct BufferCfg {
    void *data;
    int size;
} BufferCfg;

typedef struct GeometryCfg {
    BufferCfg *buffers;
    int buffer_count;
    AttributeCfg *attributes;
    int attribute_count;
    int vertex_count;
    int mode;
} GeometryCfg;

typedef struct Cabinet Cabinet;
typedef struct Gfx Gfx;
typedef struct Shader Shader;
typedef struct Geometry Geometry;
typedef struct AssetHandle AssetHandle;
typedef struct Texture Texture;
typedef struct Uniforms Uniforms;

extern Cabinet *cabinet_create();
extern void cabinet_destroy(Cabinet *cabinet);
extern AssetHandle *cabinet_load_image(Cabinet *cabinet, const char *path);
extern bool cabinet_is_loaded(Cabinet *cabinet, AssetHandle *handle);

extern Gfx *gfx_create(Cabinet *cabinet);
extern void gfx_destroy(Gfx *gfx);
extern void gfx_clear(Gfx *gfx, float r, float g, float b, float a);
extern Shader *gfx_create_shader(Gfx *gfx, const char *vertex, const char *fragment);
extern void gfx_bind_shader(Gfx *gfx, Shader *shader);
extern void gfx_bind_texture(Gfx *gfx, Texture *texture);
extern Geometry *gfx_create_geometry(Gfx *gfx, GeometryCfg *cfg);
extern void gfx_draw_geometry(Gfx *gfx, Geometry *geometry);
extern Texture *gfx_create_texture(Gfx *gfx, AssetHandle *handle);
extern Texture *gfx_create_texture_array(Gfx *gfx, AssetHandle *handles, int count);
extern Uniforms *gfx_create_uniforms(Gfx *gfx, char *name, int size);
extern void gfx_update_uniforms(Gfx *gfx, Uniforms *uniforms, void *data, int size);
extern void gfx_set_vertex_count(Geometry *geometry, int vertex_count);
extern void gfx_update_geometry(Gfx *gfx, Geometry *geometry, int bufferIdx, void *data, int size);

extern void shader_destroy(Shader *shader);

extern void log_info(Cabinet *cabinet, const char *msg);
extern float cab_sin(float x);
extern float cab_cos(float x);
extern float cab_tan(float x);
extern float cab_acos(float x);
extern float cab_asin(float x);
extern float cab_sqrt(float x);

#endif
