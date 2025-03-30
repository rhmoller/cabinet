@vs vs
in vec3 a_pos;
in vec4 a_color;
out vec4 v_color;

void main() {
  gl_Position = vec4(a_pos, 1.0);
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

@program triangle vs fs

