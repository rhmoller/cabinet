#include "shaders.h"

const char *vtx_shader = "\
#version 300 es\n\
layout(std140) uniform Globals {\n\
    mat4 model;\n\
    mat4 view;\n\
    mat4 projection;\n\
} globals;\n\
in vec3 aPos;\n\
in vec3 aCol;\n\
in vec2 aTexCoord;\n\
out vec3 vCol;\n\
out vec2 vTexCoord;\n\
void main() {\n\
    gl_Position = globals.projection * globals.view * globals.model * vec4(aPos, 1);\n\
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

Shader *createStandardShader(Gfx *gfx) {
    return gfx_create_shader(gfx, vtx_shader, frag_shader);
}

const char *voxel_vtx_shader = "\
#version 300 es\n\
layout(std140) uniform Globals {\n\
    mat4 model;\n\
    mat4 view;\n\
    mat4 projection;\n\
} globals;\n\
in vec3 aPos;\n\
in vec3 aCol;\n\
in vec3 aTexCoord;\n\
in vec3 aNormal;\n\
out vec3 vCol;\n\
out vec3 vTexCoord;\n\
out vec3 vNormal;\n\
void main() {\n\
    gl_Position = globals.projection * globals.view * globals.model * vec4(aPos, 1);\n\
    vCol = aCol;\n\
    vTexCoord = aTexCoord;\
    vNormal = mat3(transpose(inverse(globals.model))) * aNormal;\
}\n\
\0";

const char *voxel_frag_shader = "\
#version 300 es\n\
precision mediump float;\n\
precision mediump sampler2DArray;\n\
uniform sampler2DArray uTexture;\n\
in vec3 vCol;\n\
in vec3 vTexCoord;\n\
in vec3 vNormal;\n\
out vec4 FragColor;\n\
void main() {\n\
    vec3 lightDir = normalize(vec3(0.25, 1.0, 0.5));\n\
    vec3 lightColor = vec3(1.0, 1.0, 1.0);\n\
    float ambientStrength = 0.3;\n\
\n\
    vec3 ambient = ambientStrength * lightColor;\n\
    vec3 norm = normalize(vNormal);\n\
    float diff = max(dot(norm, lightDir), 0.0);\n\
    vec3 diffuse = diff * lightColor;\n\
    vec3 result = (ambient + diffuse) * vCol;\n\
\n\
    FragColor = vec4(result, 1.0) * texture(uTexture, vTexCoord);\n\
}\n\
\0";


Shader *createVoxelShader(Gfx *gfx) {
    return gfx_create_shader(gfx, voxel_vtx_shader, voxel_frag_shader);
}
