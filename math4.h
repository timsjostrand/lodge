#ifndef _MATH4_H
#define _MATH4_H

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define xyz(v) v[0], v[1], v[2]
#define xyzw(v) v[0], v[1], v[2], v[3]

#define swapf(t,a,b) t = a;\
	a = b; \
	b = t;

typedef float mat4[16];
typedef float vec2[2];
typedef float vec3[3];
typedef float vec4[4];

void ortho(mat4 m, float left, float right, float top, float bottom, float near, float far);
void mult(mat4 m, const mat4 a, const mat4 b);
void mult_same(mat4 a, const mat4 b);
void mult_scalar(mat4 m, const mat4 a, const float s);
void mult_scalar_same(mat4 m, const float s);
void add(mat4 a, const mat4 b);
void identity(mat4 m);
void translate(mat4 m, float x, float y, float z);
void translatev(mat4 m, vec4 v);
void rotate_x(mat4 m, const float angle);
void rotate_y(mat4 m, const float angle);
void rotate_z(mat4 m, const float angle);
void scale(mat4 m, float x, float y, float z);
void scalev(mat4 m, vec4 v);
void transpose(mat4 m, mat4 a);
void transpose_same(mat4 m);
void transpose_same_copy(mat4 m);
void cross(vec4 v, vec4 a, vec4 b);
int inverse(mat4 m, const mat4 a);
float determinant(const mat4 m);
void adjugate(mat4 m, const mat4 a);

int imax(int a, int b);
int imin(int a, int b);
float clamp(float f, float min, float max);
float randr(float min, float max);

void printm(mat4 m);
void printv(vec4 v);

void copym(mat4 m, const mat4 a);
void copyv(vec4 v, const vec4 a);
void set2f(vec2 v, const float x, const float y);
void set3f(vec3 v, const float x, const float y, const float z);
void set4f(vec4 v, const float x, const float y, const float z, const float w);

#endif
