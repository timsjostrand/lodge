#ifndef _MATH4_H
#define _MATH4_H

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define xy_of(v) v[0], v[1]
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
void mult_vec4(vec4 v, const mat4 m, const vec4 a);
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
float distancef(float x, float y);
float distance2f(vec2 a, vec2 b);
float distance3f(const vec3 a, const vec3 b);

void add2f(vec2 dst, const float x, const float y);
void sub2f(vec2 dst, const float x, const float y);
void mult2f(vec2 dst, const float x, const float y);

void add3f(vec3 dst, const float x, const float y, const float z);
void sub3f(vec3 dst, const float x, const float y, const float z);
void mult3f(vec3 dst, const float x, const float y, const float z);

float length2f(const vec2 v);
float length3f(const vec3 v);
float length4f(const vec4 v);

float lerp1f(float min, float max, float t);
void lerp2f(vec2 dst, const vec2 src, float t);

int imax(int a, int b);
int imin(int a, int b);
float clamp(float f, float min, float max);
float randr(float min, float max);
float angle_from_to(vec2 a, vec2 b);

void printm(mat4 m);
void printv(vec4 v);

void copym(mat4 m, const mat4 a);
void copyv(vec4 v, const vec4 a);
void set2f(vec2 v, const float x, const float y);
void set3f(vec3 v, const float x, const float y, const float z);
void set4f(vec4 v, const float x, const float y, const float z, const float w);

void norm2f(vec2 v);

int sign(int x);

#endif
