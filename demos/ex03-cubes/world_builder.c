#include "world_builder.h"
#include <stdint.h>
#include "cmath.h"

// Initialize the world builder
void world_builder_init(WorldBuilder* builder, float* vertex_buffer, size_t max_vertices) {
    builder->vertex_buffer = vertex_buffer;
    builder->current_index = 0;
    builder->max_vertices = max_vertices * VERTEX_STRIDE;
}

// Internal helper to add a vertex
static void add_vertex(WorldBuilder* builder, vec3 pos, float u, float v) {
    if (builder->current_index + VERTEX_STRIDE <= builder->max_vertices) {
        float* ptr = builder->vertex_buffer + builder->current_index;
        ptr[0] = pos.x;
        ptr[1] = pos.y;
        ptr[2] = pos.z;
        ptr[3] = u;
        ptr[4] = v;
        builder->current_index += VERTEX_STRIDE;
    }
}

// Add a quad to the mesh
void world_builder_add_quad(
    WorldBuilder* builder,
    vec3 start,
    vec3 vec1,
    vec3 vec2,
    uint16_t tileIdx
) {
    // Calculate texture coordinates
    float tile_x = (float)(tileIdx % TILE_COUNT_X);
    float tile_y = floor(tileIdx / (float)TILE_COUNT_X);
    
    float u0 = (tile_x * TILE_SIZE) / TEXTURE_ATLAS_SIZE;
    float v0 = (tile_y * TILE_SIZE) / TEXTURE_ATLAS_SIZE;
    float u1 = ((tile_x + 1) * TILE_SIZE) / TEXTURE_ATLAS_SIZE;
    float v1 = ((tile_y + 1) * TILE_SIZE) / TEXTURE_ATLAS_SIZE;

    // Calculate quad corners
    vec3 corners[4] = {
        start,                                  // Corner 0
        {start.x + vec1.x, start.y + vec1.y, start.z + vec1.z}, // Corner 1
        {start.x + vec1.x + vec2.x,              // Corner 2
         start.y + vec1.y + vec2.y,
         start.z + vec1.z + vec2.z},
        {start.x + vec2.x, start.y + vec2.y, start.z + vec2.z} // Corner 3
    };

    // Add two triangles (CCW winding)
    // Triangle 1
    add_vertex(builder, corners[0], u0, v0);
    add_vertex(builder, corners[1], u1, v0);
    add_vertex(builder, corners[2], u1, v1);
    
    // Triangle 2
    add_vertex(builder, corners[0], u0, v0);
    add_vertex(builder, corners[2], u1, v1);
    add_vertex(builder, corners[3], u0, v1);
}

// Get the current vertex count
size_t world_builder_get_vertex_count(const WorldBuilder* builder) {
    return builder->current_index / VERTEX_STRIDE;
}

// Adds a cube to the mesh with the given center position, size, and tile indices for each face
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
) {
    float half_size = size * 0.5f;
    vec3 corners[8] = {
        {center.x - half_size, center.y - half_size, center.z - half_size}, // 0: left-bottom-back
        {center.x + half_size, center.y - half_size, center.z - half_size}, // 1: right-bottom-back
        {center.x + half_size, center.y + half_size, center.z - half_size}, // 2: right-top-back
        {center.x - half_size, center.y + half_size, center.z - half_size}, // 3: left-top-back

        {center.x - half_size, center.y - half_size, center.z + half_size}, // 4: left-bottom-front
        {center.x + half_size, center.y - half_size, center.z + half_size}, // 5: right-bottom-front

        {center.x + half_size, center.y + half_size, center.z + half_size}, // 6: right-top-front
        {center.x - half_size, center.y + half_size, center.z + half_size}  // 7: left-top-front
    };

    // Front face (using front_tile)
    world_builder_add_quad(builder, corners[7], 
        (vec3){size, 0, 0}, (vec3){0, -size, 0}, front_tile);
    
    // Back face (using back_tile)
    world_builder_add_quad(builder, corners[2], 
        (vec3){-size, 0, 0}, (vec3){0, -size, 0}, back_tile);
    
    // Left face (using left_tile)
    world_builder_add_quad(builder, corners[3], 
        (vec3){0, 0, size}, (vec3){0, -size, 0}, left_tile);
    
    // Right face (using right_tile)
    world_builder_add_quad(builder, corners[6], 
        (vec3){0, 0, -size}, (vec3){0, -size, 0}, right_tile);
    
    // Top face (using top_tile)
    world_builder_add_quad(builder, corners[3], 
        (vec3){size, 0, 0}, (vec3){0, 0, size}, top_tile);
    
    // Bottom face (using bottom_tile)
    world_builder_add_quad(builder, corners[4], 
        (vec3){size, 0, 0}, (vec3){0, 0, -size}, bottom_tile);
}

void world_builder_heightmap(WorldBuilder* builder, float width, float depth, float tileSize, Heightmap_Func heightmap_func) {
    float half_width = width * 0.5f;
    float half_depth = depth * 0.5f;
    for (float x = 0; x < width; x+= tileSize) {
        for (float z = 0; z < depth; z+= tileSize) {
            float x1 = x * tileSize - half_width;
            float z1 = z * tileSize - half_depth;
            float x2 = x1 + tileSize;
            float z2 = z1 + tileSize;

            int tileIdx = heightmap_func(x1, z1) < 0.0f ? 5 : 11;
            float tile_x = (float)(tileIdx % TILE_COUNT_X);
            float tile_y = floor(tileIdx / (float)TILE_COUNT_X);
            float u0 = (tile_x * TILE_SIZE) / TEXTURE_ATLAS_SIZE;
            float v0 = (tile_y * TILE_SIZE) / TEXTURE_ATLAS_SIZE;
            float u1 = ((tile_x + 1) * TILE_SIZE) / TEXTURE_ATLAS_SIZE;
            float v1 = ((tile_y + 1) * TILE_SIZE) / TEXTURE_ATLAS_SIZE;

           vec3 coords[4] = {
                { x1, heightmap_func(x1, z1), z1 },
                { x2, heightmap_func(x2, z1), z1 },
                { x2, heightmap_func(x2, z2), z2 },
                { x1, heightmap_func(x1, z2), z2 }
            };
            add_vertex(builder, coords[0], u0, v0);
            add_vertex(builder, coords[1], u1, v0);
            add_vertex(builder, coords[2], u1, v1);

            add_vertex(builder, coords[2], u1, v1);
            add_vertex(builder, coords[3], u0, v1);
            add_vertex(builder, coords[0], u0, v0);
        }
    }
}

