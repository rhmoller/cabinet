@ctype mat4 mat4

@vs vs
layout(binding=0) uniform vs_params {
    mat4 mvp;
};

in vec4 a_pos;
in vec4 a_color;

out vec4 v_color;

void main() {
    gl_Position = mvp * a_pos;
    v_color = a_color;
}
@end

@fs fs
in vec4 v_color;
out vec4 frag_color;

void main() {
    frag_color = v_color;
}
@end

@program cube vs fs
