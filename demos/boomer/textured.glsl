@ctype mat4 mat4

@vs vs
layout(binding=0) uniform vs_params {
    mat4 mvp;
};

in vec4 a_pos;
in vec2 a_texcoord;

out vec2 v_texcoord;

void main() {
    gl_Position = mvp * a_pos;
    v_texcoord = a_texcoord;
}
@end

@fs fs
in vec2 v_texcoord;
out vec4 frag_color;
layout(binding=0) uniform texture2D tex;
layout(binding=0) uniform sampler smp;

void main() {
    frag_color = texture(sampler2D(tex, smp), v_texcoord);
}
@end

@program textured vs fs
