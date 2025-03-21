#include "shapes.h"

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

Geometry *createCubeGeometry(Gfx *gfx) {
  return gfx_create_geometry(gfx, &(GeometryCfg){
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
}


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

Geometry *create_text_geometry(Gfx *gfx, float *text_vertices, int text_len) {
  return gfx_create_geometry(gfx, &(GeometryCfg){
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
}
