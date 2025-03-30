#include "shaders.h"
#include "shapes.h"
#ifndef __wasm__
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif // __wasm__

#include "cabinet.h"
#include "cmath.h"
#include "voxels.h"
#include "base.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

Cabinet *cabinet;
Gfx *gfx;
Shader *shader;
Shader *voxelShader;

Geometry *geometry;
Texture *texture;

Geometry *textGeometry;
Texture *fontTexture;

Geometry *voxelGeometry;
Texture *voxelTexture;

float voxel_vertices[8 * 36 * 1024 * 50];
float text_vertices[8 * 36 * 4096];

AssetHandle *baboon;
AssetHandle *font;
AssetHandle *voxelImg;

typedef struct UniformData {
    float model[16];
    float view[16];
    float projection[16];
} UniformData;

UniformData uniformData = {
    .model = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
   },
    .view = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
   },
    .projection = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
   },
};

Uniforms *uniforms;

mat4 identity;

void init() {
    mat4_to_identity(&identity);

    cabinet = cabinet_create();
    gfx = gfx_create(cabinet);

    voxelShader = createVoxelShader(gfx);
    shader = createStandardShader(gfx);

    gfx_bind_shader(gfx, shader);
    geometry = createCubeGeometry(gfx);

    int text_len = write_text(text_vertices, "b", 300, 32);
    textGeometry = create_text_geometry(gfx, text_vertices, text_len);
    int voxel_len = voxels_create(voxel_vertices);

    gfx_bind_shader(gfx, voxelShader);
    voxelGeometry = gfx_create_geometry(gfx, &(GeometryCfg){
        .buffers = (BufferCfg[]){
            { .data = voxel_vertices, .size = sizeof(voxel_vertices) }
        },
        .buffer_count = 1,
        .attributes = (AttributeCfg[]){
            { .name = "aPos", .buffer = 0, .size = 3, .type = CAB_FLOAT, .stride = 12 * sizeof(float), .offset = 0 },
            { .name = "aCol", .buffer = 0, .size = 3, .type = CAB_FLOAT, .stride = 12 * sizeof(float), .offset = 3 * sizeof(float) },
            { .name = "aTexCoord", .buffer = 0, .size = 3, .type = CAB_FLOAT, .stride = 12 * sizeof(float), .offset = 6 * sizeof(float) },
            { .name = "aNormal", .buffer = 0, .size = 3, .type = CAB_FLOAT, .stride = 12 * sizeof(float), .offset = 9 * sizeof(float) }
        },
        .attribute_count = 4,
        .vertex_count = voxel_len,
        .mode = CAB_TRIANGLES
    });
   
    uniforms = gfx_create_uniforms(gfx, "Globals", sizeof(UniformData));

    font = cabinet_load_image(cabinet, "vincent.png");
    baboon = cabinet_load_image(cabinet, "baboon.png");
    voxelImg = cabinet_load_image(cabinet, "blocks-array-16.png");
}

bool loaded = false;

float t = 0.0f;

char text[4096];

void update() {
    mat4 view = mat4_look_at((vec3){0.0f, 5.0f, 8.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
    mat4 projection = mat4_perspective(45.0f, 1920.0f / 1080.0f, 0.1f, 100.0f);

    if (!loaded) {
        if (cabinet_is_loaded(cabinet, baboon) && cabinet_is_loaded(cabinet, font) && cabinet_is_loaded(cabinet, voxelImg)) {
            loaded = true;
            log_info(cabinet, "Baboon and vincent loaded");
            texture = gfx_create_texture(gfx, baboon);
            fontTexture = gfx_create_texture(gfx, font);
            voxelTexture = gfx_create_texture_array(gfx, voxelImg, 4);
            log_info(cabinet, "Textures created");
        } else {
            log_info(cabinet, "Baboon or vincent not loaded");
        }
    }

    gfx_clear(gfx, 0.3f, 0.1f, 0.2f, 1.0f);
    
    // 3d render spinning cube

    mat4 model = mat4_rotation_y(t * 0.1); //mat4_multiply(mat4_rotation_z(t * 0.237f), mat4_rotation_x(t * 0.1f));
    memcpy(uniformData.model, &model, sizeof(mat4));
    memcpy(uniformData.view, &view, sizeof(mat4));
    memcpy(uniformData.projection, &projection, sizeof(mat4));
    gfx_bind_shader(gfx, shader);
    gfx_update_uniforms(gfx, uniforms, &uniformData, sizeof(UniformData));

    int voxel_len = voxels_create(voxel_vertices);
    gfx_update_geometry(gfx, voxelGeometry, 0, voxel_vertices, 9 * voxel_len * sizeof(float));
    gfx_set_vertex_count(voxelGeometry, voxel_len);

    gfx_bind_shader(gfx, voxelShader);
    gfx_bind_texture(gfx, voxelTexture);
    gfx_draw_geometry(gfx, voxelGeometry);

    gfx_bind_shader(gfx, shader);
    gfx_bind_texture(gfx, texture);
    gfx_draw_geometry(gfx, geometry);

    // 2d render text
    mat4 textModel = mat4_ortho(0.0f, 1920.0f, 0.0f, 1080.0f, -1.0f, 1.0f);
    memcpy(uniformData.model, &identity, sizeof(mat4));
    memcpy(uniformData.view, &identity, sizeof(mat4));
    memcpy(uniformData.projection, &textModel, sizeof(mat4));
    gfx_bind_shader(gfx, shader);
    gfx_update_uniforms(gfx, uniforms, &uniformData, sizeof(UniformData));

    stbsp_sprintf(text, "Brain Monitor v2.0\nLost braincells: { %f }\n\nConclusion: You look like a monkey", t);
    int text_len = write_text(text_vertices, text, 32, 32);
    gfx_update_geometry(gfx, textGeometry, 0, text_vertices, 6 * text_len * 8 * sizeof(float));
    gfx_set_vertex_count(textGeometry, 6 * text_len);

    gfx_bind_texture(gfx, fontTexture);
    gfx_draw_geometry(gfx, textGeometry);

    t += 0.11111f;
}

void shutdown() {
    gfx_destroy(gfx);
    cabinet_destroy(cabinet);
}

#ifndef __wasm__
int main() {
    GLFWwindow *window;
    if (!glfwInit()) return -1;
    window = glfwCreateWindow(1920, 1080, "Cabinet", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, 1920, 1080);
    init();
    while (!glfwWindowShouldClose(window)) {
        update();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    shutdown();
    glfwTerminate();
    return 0;
}
#endif // __wasm__
