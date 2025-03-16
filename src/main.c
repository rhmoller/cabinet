#ifndef __wasm__
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif // __wasm__
#include "cabinet.h"
#include "cmath.h"
#include "voxels.h"

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

const char *vtx_shader = "\
#version 300 es\n\
layout(std140) uniform Globals {\n\
    mat4 model;\n\
} globals;\n\
in vec3 aPos;\n\
in vec3 aCol;\n\
in vec2 aTexCoord;\n\
out vec3 vCol;\n\
out vec2 vTexCoord;\n\
void main() {\n\
    gl_Position = globals.model * vec4(aPos, 1);\n\
    vCol = aCol;\n\
    vTexCoord = aTexCoord;\
}\n\
\0";

const char *frag_shader = "\
#version 300 es\n\
uniform sampler2D uTexture;\n\
precision mediump float;\n\
in vec3 vCol;\n\
in vec2 vTexCoord;\n\
out vec4 FragColor;\n\
void main() {\n\
    FragColor = vec4(vCol, 1.0) * texture(uTexture, vTexCoord);\n\
}\n\
\0";

const char *voxel_vtx_shader = "\
#version 300 es\n\
layout(std140) uniform Globals {\n\
    mat4 model;\n\
} globals;\n\
in vec3 aPos;\n\
in vec3 aCol;\n\
in vec3 aTexCoord;\n\
out vec3 vCol;\n\
out vec3 vTexCoord;\n\
void main() {\n\
    gl_Position = globals.model * vec4(aPos, 1);\n\
    vCol = aCol;\n\
    vTexCoord = aTexCoord;\
}\n\
\0";

const char *voxel_frag_shader = "\
#version 300 es\n\
precision mediump float;\n\
precision mediump sampler2DArray;\n\
uniform sampler2DArray uTexture;\n\
in vec3 vCol;\n\
in vec3 vTexCoord;\n\
out vec4 FragColor;\n\
void main() {\n\
    FragColor = vec4(vCol, 1.0) * texture(uTexture, vTexCoord);\n\
}\n\
\0";


// vertices for a cube with rgb colors and uv coordinates
float vertices[] = {
    // Positions          // Colors           // UVs
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f
};

float text_vertices[8 * 36 * 4096];
float voxel_vertices[8 * 36 * 1024];

AssetHandle *baboon;
AssetHandle *font;
AssetHandle *voxelImg;

typedef struct UniformData {
    float model[16];
} UniformData;

UniformData uniformData = {
    .model = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
   }
};

Uniforms *uniforms;

void set_vertex(float *vertices, int idx, float x, float y, float z, float r, float g, float b, float u, float v) {
    vertices[idx * 8 + 0] = x;
    vertices[idx * 8 + 1] = y;
    vertices[idx * 8 + 2] = z;
    vertices[idx * 8 + 3] = r;
    vertices[idx * 8 + 4] = g;
    vertices[idx * 8 + 5] = b;
    vertices[idx * 8 + 6] = u;
    vertices[idx * 8 + 7] = v;
}

int write_text(float *vertices, char *text, float xStart, float yStart) {
    int idx = 0;
    char c;

    float x = xStart;
    float y = 600.0f - yStart;
    float nChars = 128.0f;
    float tw = 1.0f; // width of one character in texture coordinates
    float th = 1.0f / nChars; // height of one character in texture coordinates
    float sz = 16.0f; // character size on screen in pixels

    int count = 0;

    while ((c = text[idx]) != 0) {
        int ascii = (int)c;
        if (ascii == 10 || ascii == 13) {
            x = xStart;
            y -= sz;
            idx++;
            count++;
            continue;
        }
        int row = ascii;
        int col = 0;
        float u = (float)col / 1.0f;
        float v = (float)row / nChars;

        set_vertex(vertices, 0 + idx * 6, x - 0.5f * sz, y + 0.5f * sz, 0.0f, 1.0f, 1.0f, 1.0f, u, v);
        set_vertex(vertices, 1 + idx * 6, x + 0.5f * sz, y + 0.5f * sz, 0.0f, 1.0f, 1.0f, 1.0f, u + tw, v);
        set_vertex(vertices, 2 + idx * 6, x + 0.5f * sz, y - 0.5f * sz, 0.0f, 1.0f, 1.0f, 1.0f, u + tw, v + th);

        set_vertex(vertices, 3 + idx * 6, x + 0.5f * sz, y - 0.5f * sz, 0.0f, 1.0f, 1.0f, 1.0f, u + tw, v + th);
        set_vertex(vertices, 4 + idx * 6, x - 0.5f * sz, y - 0.5f * sz, 0.0f, 1.0f, 1.0f, 1.0f, u, v + th);
        set_vertex(vertices, 5 + idx * 6, x - 0.5f * sz, y + 0.5f * sz, 0.0f, 1.0f, 1.0f, 1.0f, u, v);

        x += sz;
        idx++;
        count++;
   }

   return count;
}

void init() {
    cabinet = cabinet_create();
    gfx = gfx_create(cabinet);
    voxelShader = gfx_create_shader(gfx, voxel_vtx_shader, voxel_frag_shader);
    shader = gfx_create_shader(gfx, vtx_shader, frag_shader);
    gfx_bind_shader(gfx, shader);
    geometry = gfx_create_geometry(gfx, &(GeometryCfg){
        .buffers = (BufferCfg[]){
            { .data = vertices, .size = sizeof(vertices) }
        },
        .buffer_count = 1,
        .attributes = (AttributeCfg[]){
            { .name = "aPos", .buffer = 0, .size = 3, .type = CAB_FLOAT, .stride = 8 * sizeof(float), .offset = 0 },
            { .name = "aCol", .buffer = 0, .size = 3, .type = CAB_FLOAT, .stride = 8 * sizeof(float), .offset = 3 * sizeof(float) },
            { .name = "aTexCoord", .buffer = 0, .size = 2, .type = CAB_FLOAT, .stride = 8 * sizeof(float), .offset = 6 * sizeof(float) }
        },
        .attribute_count = 3,
        .vertex_count = 36,
        .mode = CAB_TRIANGLES
    });

    int text_len = write_text(text_vertices, "b", 300, 32);

    textGeometry = gfx_create_geometry(gfx, &(GeometryCfg){
        .buffers = (BufferCfg[]){
            { .data = text_vertices, .size = sizeof(text_vertices) }
        },
        .buffer_count = 1,
        .attributes = (AttributeCfg[]){
            { .name = "aPos", .buffer = 0, .size = 3, .type = CAB_FLOAT, .stride = 8 * sizeof(float), .offset = 0 },
            { .name = "aCol", .buffer = 0, .size = 3, .type = CAB_FLOAT, .stride = 8 * sizeof(float), .offset = 3 * sizeof(float) },
            { .name = "aTexCoord", .buffer = 0, .size = 2, .type = CAB_FLOAT, .stride = 8 * sizeof(float), .offset = 6 * sizeof(float) }
        },
        .attribute_count = 3,
        .vertex_count = 6 * text_len,
        .mode = CAB_TRIANGLES
    });

    int voxel_len = voxels_create(voxel_vertices, 0);

    voxelGeometry = gfx_create_geometry(gfx, &(GeometryCfg){
        .buffers = (BufferCfg[]){
            { .data = voxel_vertices, .size = sizeof(voxel_vertices) }
        },
        .buffer_count = 1,
        .attributes = (AttributeCfg[]){
            { .name = "aPos", .buffer = 0, .size = 3, .type = CAB_FLOAT, .stride = 9 * sizeof(float), .offset = 0 },
            { .name = "aCol", .buffer = 0, .size = 3, .type = CAB_FLOAT, .stride = 9 * sizeof(float), .offset = 3 * sizeof(float) },
            { .name = "aTexCoord", .buffer = 0, .size = 3, .type = CAB_FLOAT, .stride = 9 * sizeof(float), .offset = 6 * sizeof(float) }
        },
        .attribute_count = 3,
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
    mat4 view = mat4_look_at((vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
    mat4 projection = mat4_perspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);

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
    mat4 model = mat4_multiply(mat4_rotation_z(t * 0.237f), mat4_rotation_x(t * 0.1f));
    mat4 data = mat4_multiply(projection, mat4_multiply(view, model));
    gfx_update_uniforms(gfx, uniforms, &data, sizeof(UniformData));

    int voxel_len = voxels_create(voxel_vertices, (int)t % 10 > 5 ? 2 : 0);
    gfx_update_geometry(gfx, voxelGeometry, 0, voxel_vertices, 9 * voxel_len * sizeof(float));
    gfx_set_vertex_count(voxelGeometry, voxel_len);

    gfx_bind_shader(gfx, voxelShader);
    gfx_bind_texture(gfx, voxelTexture);
    gfx_draw_geometry(gfx, voxelGeometry);

    gfx_bind_shader(gfx, shader);
    gfx_bind_texture(gfx, texture);
    gfx_draw_geometry(gfx, geometry);

    // 2d render text
    mat4 textModel = mat4_ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
    gfx_update_uniforms(gfx, uniforms, &textModel, sizeof(UniformData));

    stbsp_sprintf(text, "Brain Monitor v2.0\nLost braincells: { %f.3 }\n\nConclusion: You look like a monkey", t);
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
    window = glfwCreateWindow(800, 600, "Cabinet", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, 800, 600);
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
