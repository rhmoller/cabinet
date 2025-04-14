#ifndef SRC_MATH_H
#define SRC_MATH_H
#include <math.h>

typedef struct {
  float x, y;
} vec2;

typedef struct {
  float x, y, z;
} vec3;

// Function to create a vec3
static inline vec3 vec3_create(float x, float y, float z) {
  return (vec3){x, y, z};
}

// Function to add two vec3s
static inline vec3 vec3_add(vec3 a, vec3 b) {
  return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

// Function to subtract two vec3s
static inline vec3 vec3_sub(vec3 a, vec3 b) {
  return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

// Function to scale a vec3 by a scalar
static inline vec3 vec3_scale(vec3 v, float scalar) {
  return (vec3){v.x * scalar, v.y * scalar, v.z * scalar};
}

// Function to compute the dot product of two vec3s
static inline float vec3_dot(vec3 a, vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Function to compute the cross product of two vec3s
static inline vec3 vec3_cross(vec3 a, vec3 b) {
  return (vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

// Function to compute the length of a vec3
static inline float vec3_length(vec3 v) { return sqrt(vec3_dot(v, v)); }

// Function to normalize a vec3
static inline vec3 vec3_normalize(vec3 v) {
  float length = vec3_length(v);
  return vec3_scale(v, 1.0f / length);
}

typedef struct {
  float x, y, z, w;
} vec4;

// Function to create a vec4
static inline vec4 vec4_create(float x, float y, float z, float w) {
  return (vec4){x, y, z, w};
}

// Function to add two vec4s
static inline vec4 vec4_add(vec4 a, vec4 b) {
  return (vec4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

// Function to subtract two vec4s
static inline vec4 vec4_sub(vec4 a, vec4 b) {
  return (vec4){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

// Function to scale a vec4 by a scalar
static inline vec4 vec4_scale(vec4 v, float scalar) {
  return (vec4){v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar};
}

// Function to compute the dot product of two vec4s
static inline float vec4_dot(vec4 a, vec4 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

// Function to compute the cross product of two vec4s
static inline vec4 vec4_cross(vec4 a, vec4 b) {
  return (vec4){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x, 0.0f};
}

// Function to compute the length of a vec4
static inline float vec4_length(vec4 v) { return sqrt(vec4_dot(v, v)); }

// Function to normalize a vec4
static inline vec4 vec4_normalize(vec4 v) {
  float length = vec4_length(v);
  return vec4_scale(v, 1.0f / length);
}

typedef struct {
  float elements[4][4]; // Column-major order
} mat4;

// Function to create a new mat4 (identity matrix)
static inline mat4 mat4_create() {
  return (mat4){
      .elements = {
          {1.0f, 0.0f, 0.0f, 0.0f},
          {0.0f, 1.0f, 0.0f, 0.0f},
          {0.0f, 0.0f, 1.0f, 0.0f},
          {0.0f, 0.0f, 0.0f, 1.0f},
      },
  };
}

static inline void mat4_to_identity(mat4 *out) {
  out->elements[0][0] = 1.0f;
  out->elements[0][1] = 0.0f;
  out->elements[0][2] = 0.0f;
  out->elements[0][3] = 0.0f;
  out->elements[1][0] = 0.0f;
  out->elements[1][1] = 1.0f;
  out->elements[1][2] = 0.0f;
  out->elements[1][3] = 0.0f;
  out->elements[2][0] = 0.0f;
  out->elements[2][1] = 0.0f;
  out->elements[2][2] = 1.0f;
  out->elements[2][3] = 0.0f;
  out->elements[3][0] = 0.0f;
  out->elements[3][1] = 0.0f;
  out->elements[3][2] = 0.0f;
  out->elements[3][3] = 1.0f;
}

// Function to multiple two mat4s
static inline void mat4_to_multiply(mat4 *out, mat4 *a, mat4 *b) {
  for (int col = 0; col < 4; col++) {
    for (int row = 0; row < 4; row++) {
      float sum = 0.0f;
      for (int i = 0; i < 4; i++) {
        sum += a->elements[i][row] * b->elements[col][i];
      }
      out->elements[col][row] = sum;
    }
  }
}

// Function to multiply two mat4s
static inline mat4 mat4_multiply(mat4 a, mat4 b) {
  mat4 result = mat4_create();
  mat4_to_multiply(&result, &a, &b);
  return result;
}

static inline void mat4_to_transpose(mat4 *out, mat4 *m) {
  for (int col = 0; col < 4; col++) {
    for (int row = 0; row < 4; row++) {
      out->elements[row][col] = m->elements[col][row];
    }
  }
}

// Function to transpose a mat4
static inline mat4 mat4_transpose(mat4 m) {
  mat4 result = mat4_create();
  mat4_to_transpose(&result, &m);
  return result;
}

// Function to create a rotation matrix around the x-axis
static inline void mat4_to_rotation_x(mat4 *out, float angle) {
  float c = cos(angle);
  float s = sin(angle);
  mat4_to_identity(out);
  out->elements[1][1] = c;
  out->elements[1][2] = -s;
  out->elements[2][1] = s;
  out->elements[2][2] = c;
}

// Function to create a rotation matrix around the x-axis
static inline mat4 mat4_rotation_x(float angle) {
  mat4 result = mat4_create();
  mat4_to_rotation_x(&result, angle);
  return result;
}

// Function to create a rotation matrix around the y-axis
static inline void mat4_to_rotation_y(mat4 *out, float angle) {
  float c = cos(angle);
  float s = sin(angle);
  mat4_to_identity(out);
  out->elements[0][0] = c;
  out->elements[0][2] = s;
  out->elements[2][0] = -s;
  out->elements[2][2] = c;
}


// Function to create a rotation matrix around the y-axis
static inline mat4 mat4_rotation_y(float angle) {
  mat4 result = mat4_create();
  mat4_to_rotation_y(&result, angle);
  return result;
}

// Function to create a rotation matrix around the z-axis
static inline void mat4_to_rotation_z(mat4 *out, float angle) {
  float c = cos(angle);
  float s = sin(angle);
  mat4_to_identity(out);
  out->elements[0][0] = c;
  out->elements[0][1] = -s;
  out->elements[1][0] = s;
  out->elements[1][1] = c;
}

// Function to create a rotation matrix around the z-axis
static inline mat4 mat4_rotation_z(float angle) {
  mat4 result = mat4_create();
  mat4_to_rotation_z(&result, angle);
  return result;
}

static inline mat4 mat4_rotate_x(mat4 m, float angle) {
  mat4 rotation = mat4_rotation_x(angle);
  return mat4_multiply(m, rotation);
}

static inline mat4 mat4_rotate_y(mat4 m, float angle) {
  mat4 rotation = mat4_rotation_y(angle);
  return mat4_multiply(m, rotation);
}

// Function to create a look-at matrix
static inline void mat4_to_look_at(mat4 *out, vec3 eye, vec3 center, vec3 up) {
  vec3 f = vec3_normalize(vec3_sub(center, eye));
  vec3 s = vec3_normalize(vec3_cross(f, up));
  vec3 u = vec3_cross(s, f);

  out->elements[0][0] = s.x;
  out->elements[1][0] = s.y;
  out->elements[2][0] = s.z;
  out->elements[0][1] = u.x;
  out->elements[1][1] = u.y;
  out->elements[2][1] = u.z;
  out->elements[0][2] = -f.x;
  out->elements[1][2] = -f.y;
  out->elements[2][2] = -f.z;
  out->elements[3][0] = -vec3_dot(s, eye);
  out->elements[3][1] = -vec3_dot(u, eye);
  out->elements[3][2] = vec3_dot(f, eye);
}

// Function to create a look-at matrix
static inline mat4 mat4_look_at(vec3 eye, vec3 center, vec3 up) {
  mat4 result = mat4_create();
  mat4_to_look_at(&result, eye, center, up);
  return result;
}

// Function to create a perspective matrix
static inline void mat4_to_perspective(mat4 *out, float fov, float aspect, float near, float far) {
  float f = 1.0f / tan(fov / 2.0f);
  mat4_to_identity(out);
  out->elements[0][0] = f / aspect;
  out->elements[1][1] = f;
  out->elements[2][2] = (far + near) / (near - far);
  out->elements[2][3] = -1.0f;
  out->elements[3][2] = (2.0f * far * near) / (near - far);
  out->elements[3][3] = 0.0f;
}


// Function to create a perspective matrix
static inline mat4 mat4_perspective(float fov, float aspect, float near, float far) {
  mat4 result = mat4_create();
  mat4_to_perspective(&result, fov, aspect, near, far);
  return result;
}

// Function to create an orthographic matrix 
static inline void mat4_to_ortho(mat4 *out, float left, float right, float bottom, float top, float near, float far) {
  mat4_to_identity(out);
  out->elements[0][0] = 2.0f / (right - left);
  out->elements[1][1] = 2.0f / (top - bottom);
  out->elements[2][2] = -2.0f / (far - near);
  out->elements[3][0] = -(right + left) / (right - left);
  out->elements[3][1] = -(top + bottom) / (top - bottom);
  out->elements[3][2] = -(far + near) / (far - near);
}

// Function to create an orthographic matrix 
static inline mat4 mat4_ortho(float left, float right, float bottom, float top, float near, float far) {
  mat4 result = mat4_create();
  mat4_to_ortho(&result, left, right, bottom, top, near, far);
  return result;
}

#endif // SRC_MATH_H
