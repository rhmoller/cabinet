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

    const vertex_t vertices[] = {
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

   state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices),
        .label = "cube-vertices"
    });

    uint16_t indices[] = {
        0, 1, 2,  0, 2, 3,
        6, 5, 4,  7, 6, 4,
        8, 9, 10,  8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
    };
    state.bind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(indices),
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .label = "cube-indices"
    });

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
                [ATTR_textured_a_texcoord].format = SG_VERTEXFORMAT_SHORT2N,
            },
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_BACK,
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
        .path = "public/baboon.png",
        .callback = fetch_callback,
        .buffer = SFETCH_RANGE(state.file_buffer),
    });
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
    state.rx += 1.0f * t; state.ry += 2.0f * t;
    mat4 proj = mat4_perspective(60.0f * 3.1456f / 180.0f, w / h, 0.1f, 10.0f);
    mat4 view = mat4_look_at((vec3){0.0f, 1.5f, 6.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
    mat4 model = mat4_rotate_y(mat4_rotate_x(mat4_create(), state.rx), state.ry);
    vs_params.mvp = mat4_multiply(mat4_multiply(proj, view), model);

    sdtx_canvas(sapp_width() / 4.0f, sapp_height() / 4.0f);
    sdtx_origin(5.0f, 5.0f);
    sdtx_font(0);
    sdtx_color1i(0xFFFFFFFF);
    sdtx_printf("Frame: %u\n", frame_count);

    float g = state.pass_action.colors[0].clear_value.g + 0.01f;
    //state.pass_action.colors[0].clear_value.g = g > 1.0f ? 0.0f : g;
    sg_begin_pass(&(sg_pass){ .action = state.pass_action, .swapchain = sglue_swapchain() });

    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(UB_vs_params, &SG_RANGE(vs_params));
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
