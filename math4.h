#ifndef _MATH4_H
#define _MATH4_H

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define xyz(v) v[0], v[1], v[2]
#define xyzw(v) v[0], v[1], v[2], v[3]

typedef float mat4[16];
typedef float vec4[4];

void ortho(mat4 m, float left, float right, float top, float bottom, float near, float far);
void mult(mat4 m, mat4 a, mat4 b);
void identity(mat4 m);
void translate(mat4 m, float x, float y, float z);
void translatev(mat4 m, vec4 v);
void rotate_z(mat4 m, float angle);
void scale(mat4 m, float x, float y, float z);
void scalev(mat4 m, vec4 v);
void transpose(mat4 m, mat4 a);
void cross(vec4 v, vec4 a, vec4 b);
int imax(int a, int b);
float clamp(float f, float min, float max);
float randr(float min, float max);

void printm(mat4 m);
void printv(vec4 v);

void copym(mat4 m, const mat4 a);
void copyv(vec4 v, const vec4 a);
void setv(vec4 v, float x, float y, float z, float w);

#endif
