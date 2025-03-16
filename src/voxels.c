#include "voxels.h"
#include "cabinet.h"

typedef struct Voxels {
  float *vertices;
  int vertex_count;
} Voxels;

void add_vertex(Voxels *voxels, float x, float y, float z,
                float r, float g,
                float b, float u, float v, float w, float nx, float ny, float nz) {
  voxels->vertices[voxels->vertex_count++] = x;
  voxels->vertices[voxels->vertex_count++] = y;
  voxels->vertices[voxels->vertex_count++] = z;
  voxels->vertices[voxels->vertex_count++] = r;
  voxels->vertices[voxels->vertex_count++] = g;
  voxels->vertices[voxels->vertex_count++] = b;
  voxels->vertices[voxels->vertex_count++] = u;
  voxels->vertices[voxels->vertex_count++] = v;
  voxels->vertices[voxels->vertex_count++] = w;
  voxels->vertices[voxels->vertex_count++] = nx;
  voxels->vertices[voxels->vertex_count++] = ny;
  voxels->vertices[voxels->vertex_count++] = nz;
}

float cube[] = {
    // clang-format off
    // Positions          // Colors           // UVs        // Normals
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,    0.0f, 0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,    0.0f, 0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,    0.0f, 0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,    0.0f, 0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,    0.0f, 0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,    0.0f, 0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,    0.0f, 0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,    0.0f, 0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,    0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,    0.0f, 0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,    -1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,    -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,    -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,    -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,    -1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,    -1.0f, 0.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,    1.0f, 0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,    1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,    1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,    1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,    1.0f, 0.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,    0.0f, -1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,    0.0f, -1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,    0.0f, -1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,    0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,    0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,    0.0f, -1.0f, 0.0f,

    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,    0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,    0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,    0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,    0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,    0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,    0.0f, 1.0f, 0.0f,
    // clang-format on
};

void insert_cube(Voxels *voxels, float x, float y, float z, float type) {
  for (int i = 0; i < sizeof(cube) / sizeof(float); i += 11) {
    add_vertex(voxels, 
               cube[i] + x, cube[i + 1] + y, cube[i + 2] + z,
               cube[i + 3], cube[i + 4], cube[i + 5],
               cube[i + 6], cube[i + 7], type,
               cube[i + 8], cube[i + 9], cube[i + 10]);
  }
}

int voxels_create(float *vertices) {
  Voxels voxels = {vertices, 0};
  for (int x = 0; x < 16; x++) {
    for (int z = 0; z < 16; z++) {
      int y = 2 + cab_cos(x * 0.5f) * cab_sin(z * 0.5f) * 2;
      int type = y == 0 ? 3 : y == 3 ? 1 : 0;
      insert_cube(&voxels, x - 8, y, z - 8, type);
    }
  }
  return voxels.vertex_count;
}
