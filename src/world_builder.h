#ifndef WORLD_BUILDER_H
#define WORLD_BUILDER_H
#include <stddef.h>
#include <stdint.h>
#include "cmath.h"

#define TEXTURE_ATLAS_SIZE 256.0f
#define TILE_COUNT_X 16
#define TILE_COUNT_Y 16
#define TILE_SIZE (TEXTURE_ATLAS_SIZE / TILE_COUNT_X)
#define VERTEX_STRIDE 5  // x,y,z,u,v

typedef struct {
    float* vertex_buffer;  // Pointer to interleaved vertex data [x,y,z,u,v,...]
    size_t current_index;  // Current byte position (in floats)
    size_t max_vertices;   // Maximum number of vertices
} WorldBuilder;

void world_builder_init(WorldBuilder* builder, float* vertex_buffer, size_t max_vertices);

static void add_vertex(WorldBuilder* builder, vec3 pos, float u, float v);

void world_builder_add_quad(
    WorldBuilder* builder,
    vec3 start,
    vec3 vec1,
    vec3 vec2,
    uint16_t tileIdx
);

size_t world_builder_get_vertex_count(const WorldBuilder* builder);
void world_builder_add_cube(
    WorldBuilder* builder,
    vec3 center,
    float size,
    uint16_t front_tile,
    uint16_t back_tile,
    uint16_t left_tile,
    uint16_t right_tile,
    uint16_t top_tile,
    uint16_t bottom_tile
);
#endif // WORLD_BUILDER_H
