#define SOKOL_IMPL
#define STB_IMAGE_IMPLEMENTATION

#include "sokol_app.h"
#include "sokol_fetch.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "sokol_debugtext.h"
#include "stb_image.h"

#include "cmath.h"
#include "textured.glsl.h"

#include <stdint.h>

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
    float tile_y = (float)(tileIdx / TILE_COUNT_X);
    
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

static struct {
    float rx;
    float ry;
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
    uint8_t file_buffer[1024 * 256];
} state;

typedef struct {
    float x, y, z;
    int16_t u, v;
} vertex_t;

static void fetch_callback(const sfetch_response_t *fetch);

static void printf_wrapper(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    sdtx_vprintf(fmt, args);
    va_end(args);
}

float vertices[5 * 6 * 6 * 5000];


    const vertex_t vertices_old[] = {
        // pos                  uvs
        { -1.0f, -1.0f, -1.0f,      0,     0 },
        {  1.0f, -1.0f, -1.0f,  32767,     0 },
        {  1.0f,  1.0f, -1.0f,  32767, 32767 },
        { -1.0f,  1.0f, -1.0f,      0, 32767 },

        { -1.0f, -1.0f,  1.0f,      0,     0 },
        {  1.0f, -1.0f,  1.0f,  32767,     0 },
        {  1.0f,  1.0f,  1.0f,  32767, 32767 },
        { -1.0f,  1.0f,  1.0f,      0, 32767 },

        { -1.0f, -1.0f, -1.0f,      0,     0 },
        { -1.0f,  1.0f, -1.0f,  32767,     0 },
        { -1.0f,  1.0f,  1.0f,  32767, 32767 },
        { -1.0f, -1.0f,  1.0f,      0, 32767 },

        {  1.0f, -1.0f, -1.0f,      0,     0 },
        {  1.0f,  1.0f, -1.0f,  32767,     0 },
        {  1.0f,  1.0f,  1.0f,  32767, 32767 },
        {  1.0f, -1.0f,  1.0f,      0, 32767 },

        { -1.0f, -1.0f, -1.0f,      0,     0 },
        { -1.0f, -1.0f,  1.0f,  32767,     0 },
        {  1.0f, -1.0f,  1.0f,  32767, 32767 },
        {  1.0f, -1.0f, -1.0f,      0, 32767 },

        { -1.0f,  1.0f, -1.0f,      0,     0 },
        { -1.0f,  1.0f,  1.0f,  32767,     0 },
        {  1.0f,  1.0f,  1.0f,  32767, 32767 },
        {  1.0f,  1.0f, -1.0f,      0, 32767 },
    };

    uint16_t indices[] = {
        0, 1, 2,  0, 2, 3,
        6, 5, 4,  7, 6, 4,
        8, 9, 10,  8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
    };

WorldBuilder builder = { vertices, 0, sizeof(vertices) / sizeof(vertex_t) };
 
void init() {
    sg_setup(&(sg_desc) {
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    sfetch_setup(&(sfetch_desc_t) {
        .max_requests = 1,
        .num_channels = 1,
        .num_lanes = 1,
        .logger.func = slog_func,
    });

    sdtx_setup(&(sdtx_desc_t) {
        .fonts = {
            [0] = sdtx_font_oric(),
        },
        .logger.func = slog_func,
    });

   state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        // .data = SG_RANGE(vertices),
        .size = sizeof(vertices),
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_DYNAMIC,
        .label = "cube-vertices"
    });

   // state.bind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
   //      // .data = SG_RANGE(indices),
   //      .size = sizeof(indices),
   //      .type = SG_BUFFERTYPE_INDEXBUFFER,
   //      .usage = SG_USAGE_DYNAMIC,
   //      .label = "cube-indices"
   //  });

    state.bind.images[IMG_tex] = sg_alloc_image();
    state.bind.samplers[SMP_smp] = sg_make_sampler(&(sg_sampler_desc) {
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_REPEAT,
        .wrap_v = SG_WRAP_REPEAT,
        .label = "cube-sampler",
    });

    sg_shader shd = sg_make_shader(textured_shader_desc(sg_query_backend()));

    state.pip = sg_make_pipeline(&(sg_pipeline_desc) {
        .shader = shd,
        .layout = {
            .attrs = {
                [ATTR_textured_a_pos].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_textured_a_texcoord].format = SG_VERTEXFORMAT_FLOAT2,
            },
        },
        .cull_mode = SG_CULLMODE_NONE,
        .depth = {
            .write_enabled = true,
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
        },
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .label = "cube-pipeline",
    });

    state.pass_action = (sg_pass_action) {
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = {0.0f, 0.0f, 0.0f, 1.0f},
        },
    };

    char path_buf[512];
    sfetch_send(&(sfetch_request_t) {
        .path = "neo16.png",
        .callback = fetch_callback,
        .buffer = SFETCH_RANGE(state.file_buffer),
    });

    world_builder_add_cube(&builder, (vec3){0.0f, 0.0f, 0.0f}, 1.0f, 0, 1, 2, 3, 4, 5);
}

static void fetch_callback(const sfetch_response_t *fetch) {
    if (fetch->fetched) {
        int w, h, n;
        uint8_t *data = stbi_load_from_memory(fetch->data.ptr, fetch->data.size, &w, &h, &n, 4);
        if (data) {
            sg_init_image(state.bind.images[IMG_tex], &(sg_image_desc) {
                .width = w,
                .height = h,
                .pixel_format = SG_PIXELFORMAT_RGBA8,
                .data.subimage[0][0] = {
                    .ptr = data,
                    .size = (size_t)(w * h * 4),
                },
                .label = "cube-image",
            });
         state.pass_action = (sg_pass_action) {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = {0.0f, 0.2f, 0.0f, 1.0f},
            },
        };
            stbi_image_free(data);
        }
    } else {
        state.pass_action = (sg_pass_action) {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = {1.0f, 0.0f, 0.0f, 1.0f},
            },
        };
    }
}

void update() {
    sfetch_dowork();

    uint32_t frame_count = sapp_frame_count();
    double frame_time = sapp_frame_duration() * 1000.0; 

    vs_params_t vs_params;
    const float w = sapp_widthf();
    const float h = sapp_heightf();
    const float t = (float)sapp_frame_duration();
    state.rx += 1.2f * t; state.ry += 1.7f * t;
    mat4 proj = mat4_perspective(60.0f * 3.1456f / 180.0f, w / h, 0.1f, 10.0f);
    mat4 view = mat4_look_at((vec3){0.0f, 1.5f, 6.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
    mat4 model = mat4_rotate_y(mat4_rotate_x(mat4_create(), state.rx), state.ry);
    vs_params.mvp = mat4_multiply(mat4_multiply(proj, view), model);

    sdtx_canvas(sapp_width() / 4.0f, sapp_height() / 4.0f);
    sdtx_origin(5.0f, 5.0f);
    sdtx_font(0);
    sdtx_color1i(0xFFFFFFFF);
    sdtx_printf("Frame: %u\n", frame_count);

    sg_update_buffer(state.bind.vertex_buffers[0], SG_RANGE_REF(vertices));
    sg_update_buffer(state.bind.index_buffer, SG_RANGE_REF(indices));

    float g = state.pass_action.colors[0].clear_value.g + 0.01f;
    //state.pass_action.colors[0].clear_value.g = g > 1.0f ? 0.0f : g;
    sg_begin_pass(&(sg_pass){ .action = state.pass_action, .swapchain = sglue_swapchain() });

    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(UB_vs_params, SG_RANGE_REF(vs_params));
    sg_draw(0, 36, 1);

    sdtx_draw();

    sg_end_pass();
    sg_commit();
}

void cleanup() {
    sg_shutdown();
}

void handle_event(const sapp_event* event) {
    // todo: handle events
}

sapp_desc sokol_main(int argc, char* argv[]) {
    return (sapp_desc) {
        .init_cb = init,
        .frame_cb = update,
        .cleanup_cb = cleanup,
        .event_cb = handle_event,
        .width = 1920 / 4,
        .height = 1080 / 4,
        .window_title = "Sokol App",
        .logger.func = slog_func,
    };
}
