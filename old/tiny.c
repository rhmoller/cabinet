#include <math.h>       // For sinf, cosf
#include <GLES2/gl2.h>  // OpenGL ES 2.0 headers
#include <emscripten.h> // Emscripten runtime functions
#include <emscripten/html5.h> // Emscripten HTML5 API (for WebGL context)

// Shader sources (minimal)
const GLchar* vertex_shader_source =
    "attribute vec3 aPosition;"
    "uniform mat4 uMVPMatrix;"
    "void main() {"
    "  gl_Position = uMVPMatrix * vec4(aPosition, 1.0);"
    "}";

const GLchar* fragment_shader_source =
    "precision mediump float;" // Required qualifier in fragment shaders
    "void main() {"
    "  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);" // White color
    "}";

// Global variables (minimize where possible)
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE gl_context;
GLuint shader_program;
GLint pos_attrib_location;
GLint mvp_uniform_location;
GLuint vertex_buffer;
GLuint index_buffer;
float rot_angle_x = 0.0f;
float rot_angle_y = 0.0f;

// Cube data (vertices and line indices)
GLfloat vertices[] = {
    // Front face
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    // Back face
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f
};

// Indices for drawing lines (12 edges)
GLushort indices[] = {
    0, 1, 1, 2, 2, 3, 3, 0, // Front face
    4, 5, 5, 6, 6, 7, 7, 4, // Back face
    0, 4, 1, 5, 2, 6, 3, 7  // Connecting lines
};

// --- Minimal Matrix Math ---
// (Inlined or static to potentially help optimizer)

// mat4 * mat4 -> result (OpenGL column-major)
static void mat4_multiply(float* restrict result, const float* restrict a, const float* restrict b) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result[i*4 + j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                result[i*4 + j] += a[k*4 + j] * b[i*4 + k];
            }
        }
    }
}

// Creates an identity matrix
static void mat4_identity(float* result) {
    for(int i = 0; i < 16; ++i) result[i] = 0.0f;
    result[0] = result[5] = result[10] = result[15] = 1.0f;
}

// Creates a perspective projection matrix
static void mat4_perspective(float* result, float fovy, float aspect, float near, float far) {
    float f = 1.0f / tanf(fovy / 2.0f);
    float nf = 1.0f / (near - far);
    mat4_identity(result);
    result[0] = f / aspect;
    result[5] = f;
    result[10] = (far + near) * nf;
    result[11] = -1.0f;
    result[14] = (2.0f * far * near) * nf;
    result[15] = 0.0f;
}

// Creates a rotation matrix around X axis
static void mat4_rotate_x(float* result, float angle_rad) {
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    mat4_identity(result);
    result[5] = c;
    result[6] = s;
    result[9] = -s;
    result[10] = c;
}

// Creates a rotation matrix around Y axis
static void mat4_rotate_y(float* result, float angle_rad) {
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    mat4_identity(result);
    result[0] = c;
    result[2] = -s;
    result[8] = s;
    result[10] = c;
}
// --- End Matrix Math ---


// Compile and link shaders
static GLuint create_shader_program(const char* vs_src, const char* fs_src) {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, NULL);
    glCompileShader(vs);
    // Minimal error check (or remove for even smaller size, but harder debugging)
    GLint status;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (!status) return 0; // Failed

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (!status) { glDeleteShader(vs); return 0; } // Failed

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    // Don't need shaders anymore after linking
    glDetachShader(program, vs);
    glDetachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!status) { glDeleteProgram(program); return 0; } // Failed

    return program;
}

// Main render loop function
void render_frame(void* user_data) {
    int width, height;
    emscripten_get_canvas_element_size("#canvas", &width, &height); // Use target canvas size
//    emscripten_get_screen_size(&width, &height); // Get CSS size

    // Update rotation
    rot_angle_x += 0.01f;
    rot_angle_y += 0.015f;

    // Calculate MVP matrix
    float projection_matrix[16];
    float view_matrix[16]; // Simple view: Look from Z=3 towards origin
    float model_matrix[16];
    float rotation_x_matrix[16];
    float rotation_y_matrix[16];
    float mv_matrix[16];
    float mvp_matrix[16];

    mat4_perspective(projection_matrix, 45.0f * M_PI / 180.0f, (float)width / (float)height, 0.1f, 100.0f);

    mat4_identity(view_matrix);
    view_matrix[14] = -3.0f; // Translate camera back

    mat4_rotate_x(rotation_x_matrix, rot_angle_x);
    mat4_rotate_y(rotation_y_matrix, rot_angle_y);
    mat4_multiply(model_matrix, rotation_y_matrix, rotation_x_matrix); // Apply Y rotation then X

    mat4_multiply(mv_matrix, view_matrix, model_matrix);
    mat4_multiply(mvp_matrix, projection_matrix, mv_matrix);

    // --- Rendering ---
    glViewport(0, 0, width, height);
    glClearColor(0.2f, 0.0f, 0.5f, 1.0f); // Dark blue background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program);

    // Update MVP uniform
    glUniformMatrix4fv(mvp_uniform_location, 1, GL_FALSE, mvp_matrix);

    // Bind buffers and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(pos_attrib_location, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(pos_attrib_location);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

    // Draw the cube lines
    glDrawElements(GL_LINES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_SHORT, 0);

    // Disable attributes (good practice)
    glDisableVertexAttribArray(pos_attrib_location);
}

// Initialization
int init_webgl() {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = 0; // No alpha for background, maybe smaller
    attrs.depth = 1; // Need depth testing
    attrs.stencil = 0;
    attrs.antialias = 0; // Turn off AA for potential size/perf gain
    attrs.majorVersion = 2; // Request WebGL 2 (via GLES 3 headers if needed) or 1 (GLES 2)
    attrs.minorVersion = 0;

    // Target the canvas with id="canvas" in the HTML
    gl_context = emscripten_webgl_create_context("#canvas", &attrs);
    if (!gl_context) return 0; // Failed

    emscripten_webgl_make_context_current(gl_context);

    // Create shader program
    shader_program = create_shader_program(vertex_shader_source, fragment_shader_source);
    if (!shader_program) return 0; // Failed

    // Get attribute and uniform locations
    pos_attrib_location = glGetAttribLocation(shader_program, "aPosition");
    mvp_uniform_location = glGetUniformLocation(shader_program, "uMVPMatrix");
    if (pos_attrib_location < 0 || mvp_uniform_location < 0) return 0; // Failed

    // Create vertex buffer
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create index buffer
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    return 1; // Success
}

int main() {
    if (!init_webgl()) {
        // Ideally log error here, but skip for size
        return 1;
    }

    // Set the main loop function
    // Use 0 for fps to use browser's requestAnimationFrame
    // Pass NULL as user data
    emscripten_set_main_loop_arg(render_frame, NULL, 0, 1);

    return 0; // Should not be reached
}
