#include "cabinet.h"

#ifndef __wasm__
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
struct Shader {
    GLuint program;
};

struct Geometry {
    GLuint vao;
    int vertex_count;
    GLuint buffers[8];
};

struct Gfx {
    GLuint current_program;
};

struct AssetHandle {
    unsigned char *data;
    int width;
    int height;
    int nChannels;
};

struct Texture {
    GLuint id;
    bool array;
};

struct Uniforms {
    GLuint ubo;
};

GLuint create_shader_from_src(int type, const char *src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf("Shader compilation failed: %s\n", info_log);
    }
    return shader;
}

GLuint create_program_from_src(const char *vertex, const char *fragment) {
    GLuint vtx_shader = create_shader_from_src(GL_VERTEX_SHADER, vertex);
    GLuint frag_shader = create_shader_from_src(GL_FRAGMENT_SHADER, fragment);
    GLuint program = glCreateProgram();
    glAttachShader(program, vtx_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, NULL, info_log);
        printf("Program linking failed: %s\n", info_log);
    }
    glDeleteShader(vtx_shader);
    glDeleteShader(frag_shader);
    return program;
}

struct Cabinet {
    int dummy;
};

Cabinet *cabinet_create() {
    return (Cabinet *)0x1;
}

void cabinet_destroy(Cabinet *cabinet) {
    (void)cabinet;
}

AssetHandle *cabinet_load_image(Cabinet *cabinet, const char *path) {
    (void)cabinet;
    (void)path;

    int width, height, channels;
    unsigned char *data = stbi_load(path, &width, &height, &channels, 0);

    AssetHandle *handle = (AssetHandle *)malloc(sizeof(AssetHandle));
    handle->data = data;
    handle->width = width;
    handle->height = height;
    handle->nChannels = channels;

    return handle;
}

bool cabinet_is_loaded(Cabinet *cabinet, AssetHandle *handle) {
    (void)cabinet;
    (void)handle;
    return true;
}

void log_info(Cabinet *cabinet, const char *msg) {
    (void)cabinet;
    printf("INFO: %s\n", msg);
}

float cab_sin(float x) {
    return sinf(x);
}

float cab_cos(float x) {
    return cosf(x);
}

float cab_tan(float x) {
    return tanf(x);
}

float cab_acos(float x) {
    return acosf(x);
}

float cab_sqrt(float x) {
    return sqrtf(x);
}

Gfx *gfx_create(Cabinet *cabinet) {
    (void)cabinet;
    Gfx* gfx = (Gfx *)malloc(sizeof(Gfx));
    gfx->current_program = 0;
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return gfx;
}

void gfx_destroy(Gfx *gfx) {
    (void)gfx;
}

void gfx_clear(Gfx *gfx, float r, float g, float b, float a) {
    (void)gfx;
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

Shader *gfx_create_shader(Gfx *gfx, const char *vertex, const char *fragment) {
    (void)gfx;
    Shader *shader = (Shader *)malloc(sizeof(Shader));
    shader->program = create_program_from_src(vertex, fragment);
    return shader;
}

void gfx_bind_shader(Gfx *gfx, Shader *shader) {
    (void)gfx;
    glUseProgram(shader->program);
    gfx->current_program = shader->program;
}

Geometry *gfx_create_geometry(Gfx *gfx, GeometryCfg *cfg) {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, cfg->buffers[0].size, cfg->buffers[0].data, GL_STATIC_DRAW);

    GLuint program = gfx->current_program;

    for (int i = 0; i < cfg->attribute_count; i++) {
        AttributeCfg *attr = &cfg->attributes[i];
        GLuint location = glGetAttribLocation(program, attr->name);
        glEnableVertexAttribArray(location);
        int type = GL_FLOAT;
        glVertexAttribPointer(location, attr->size, type, GL_FALSE, attr->stride, (void *)(uintptr_t)attr->offset);
    }

    Geometry *geometry = (Geometry *)malloc(sizeof(Geometry));
    geometry->vao = vao;
    geometry->vertex_count = cfg->vertex_count;
    geometry->buffers[0] = vbo;
    return geometry;
}

Uniforms *gfx_create_uniforms(Gfx *gfx, char *name, int size) {
    (void)gfx;
    (void)name;
    (void)size;
    GLuint program = gfx->current_program;
    GLuint location = glGetUniformBlockIndex(program, name);
    GLint block_size;
    glGetActiveUniformBlockiv(program, location, GL_UNIFORM_BLOCK_DATA_SIZE, &block_size);
    GLubyte *block_buffer = (GLubyte *)malloc(block_size); 
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, block_size, block_buffer, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, location, ubo);
 
    Uniforms *uniforms = (Uniforms *)malloc(sizeof(Uniforms));
    uniforms->ubo = ubo;
    return uniforms;
}

void gfx_update_uniforms(Gfx *gfx, Uniforms *uniforms, void *data, int size) {
    (void)gfx;
    (void)uniforms;
    (void)data;
    (void)size;
    glBindBuffer(GL_UNIFORM_BUFFER, uniforms->ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
}

void gfx_update_geometry(Gfx *gfx, Geometry *geometry, int bufferIdx, void *data, int size) {
    (void)gfx;
    glBindVertexArray(geometry->vao);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->buffers[bufferIdx]);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

void gfx_set_vertex_count(Geometry *geometry, int vertex_count) {
    geometry->vertex_count = vertex_count;
}

void gfx_draw_geometry(Gfx *gfx, Geometry *geometry) {
    (void)gfx;
    glBindVertexArray(geometry->vao);
    glDrawArrays(GL_TRIANGLES, 0, geometry->vertex_count);
}

void gfx_bind_texture(Gfx *gfx, Texture *texture) {
    (void)gfx;
    GLuint target = texture->array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    if (texture == nullptr || texture->id == 0) {
        log_info(nullptr, "Texture is null");
        glBindTexture(target, 0);
        return;
    }
    glBindTexture(target, texture->id);
}

Texture *gfx_create_texture(Gfx *gfx, AssetHandle *handle) {
    (void)gfx;
    (void)handle;
    unsigned int textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    GLenum format;
    if (handle->nChannels == 1) {
        format = GL_RED;
    } else if (handle->nChannels == 3) {
        format = GL_RGB;
    } else if (handle->nChannels == 4) {
        format = GL_RGBA;
    } else {
        log_info(nullptr, "Unsupported number of channels");
        return nullptr;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, format, handle->width, handle->height, 0, format, GL_UNSIGNED_BYTE, handle->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    log_info(nullptr, "Texture created");

    Texture *texture = (Texture *)malloc(sizeof(Texture));
    texture->id = textureId;
    return texture;
}

Texture *gfx_create_texture_array(Gfx *gfx, AssetHandle *handle, int count) {
    (void)gfx;
    (void)handle;
    (void)count;
    unsigned int textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureId);
    GLenum format;
    if (handle->nChannels == 1) {
        format = GL_RED;
    } else if (handle->nChannels == 3) {
        format = GL_RGB;
    } else if (handle->nChannels == 4) {
        format = GL_RGBA;
    } else {
        log_info(nullptr, "Unsupported number of channels");
        return nullptr;
    }
    int width = handle->width;
    int height = handle->height / count;
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, width, height, count, 0, format, GL_UNSIGNED_BYTE, nullptr);
    for (int i = 0; i < count; i++) {
        size_t offset = i * width * height * handle->nChannels;
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, format, GL_UNSIGNED_BYTE, handle->data + offset);
    }
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    Texture *texture = (Texture *)malloc(sizeof(Texture));
    texture->id = textureId;
    texture->array = true;
    return texture;
}

#endif // __wasm__
