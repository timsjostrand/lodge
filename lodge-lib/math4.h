#ifndef _MATH4_H
#define _MATH4_H

#include <stdint.h>
#include <math.h> // for M_PI
#include <stdlib.h> // for min/max

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#ifndef radians
#define radians(a)  ((a)*(M_PI/180.0f))
#endif
#ifndef degrees
#define degrees(a)  ((a)*(180.0f/M_PI))
#endif

#define xy_of(v) v.x, v.y
#define xyz_of(v) v.x, v.y, v.z
#define xyzw_of(v) v.x, v.y, v.z, v.w

#define swapf(t,a,b) t = a;\
	a = b; \
	b = t;

#ifndef min
	#define min(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef max
	#define max(x, y) ((x) > (y) ? (x) : (y))
#endif

union vec2
{
	struct { float x, y; };
	struct { float s, t; };
	float v[2];
};
typedef union vec2 vec2;

union vec3
{
	struct { float x, y, z; };
	struct { float r, g, b; };
	struct { float s, t, p; };
	float v[3];
};
typedef union vec3 vec3;

union vec4
{
	struct { float x, y, z, w; };
	struct { float r, g, b, a; };
	struct { float s, t, p, q; };
	float v[4];
};
typedef union vec4 vec4;

union mat4
{
	struct
	{
		float m00, m01, m02, m03;
		float m10, m11, m12, m13;
		float m20, m21, m22, m23;
		float m30, m31, m32, m33;
	};
	vec4 rows[4];
	float m[16];
};
typedef union mat4 mat4;

union quat
{
	struct { vec3 vec3; float s; };
	struct { float x, y, z, w; };
	vec4 vec4;
	float q[4];
};
typedef union quat quat;

struct mvp
{
	mat4 model;
	mat4 view;
	mat4 projection;
};

mat4	mat4_make(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);
mat4	mat4_make_diagonal(float s);
mat4	mat4_identity();
mat4	mat4_zero();
mat4	mat4_ones();
mat4	mat4_transpose(const mat4 m);
void	mat4_transpose_same(mat4 *m);
void	mat4_print(const mat4 m);

mat4	mat4_add(const mat4 lhs, const mat4 rhs);
mat4	mat4_mult(const mat4 lhs, const mat4 rhs);
mat4	mat4_mult_scalar(const mat4 m, const float s);
vec4	mat4_mult_vec4(const mat4 lhs, const vec4 rhs);

mat4	mat4_ortho(float left, float right, float bottom, float top, float near, float far);
mat4	mat4_frustum(float left, float right, float bottom, float top, float near, float far);
mat4	mat4_perspective(float fov, float ratio, float near, float far);
mat4	mat4_lookat(const vec3 eye_pos, const vec3 lookat_pos, const vec3 up);
mat4	mat4_look(const vec3 eye_pos, const vec3 dir, const vec3 up);

mat4	mat4_translation(float x, float y, float z);
mat4	mat4_scaling(float x, float y, float z);
mat4	mat4_rotation_x(const float angle);
mat4	mat4_rotation_y(const float angle);
mat4	mat4_rotation_z(const float angle);

mat4	mat4_rotate_x(const mat4 m, const float a);
mat4	mat4_rotate_y(const mat4 m, const float a);
mat4	mat4_rotate_z(const mat4 m, const float a);
mat4	mat4_translate(const mat4 m, float x, float y, float z);
mat4	mat4_scale(const mat4 m, float sx, float sy, float sz);
mat4	mat4_inverse(const mat4 matrix, int* is_invertible);
//mat4	mat4_adjugate(const mat4 a);
//float	mat4_determinant(const mat4 m);

vec3	mat4_view_forward(const mat4 *view);
vec3	mat4_view_strafe(const mat4 *view);

vec2	vec2_make(const float x, const float y);
vec2	vec2_zero();
vec2	vec2_add(const vec2 lhs, const vec2 rhs);
vec2	vec2_sub(const vec2 lhs, const vec2 rhs);
vec2	vec2_mult(const vec2 lhs, const vec2 rhs);
vec2	vec2_norm(const vec2 v);
vec2	vec2_lerp(const vec2 min, const vec2 max, float t);
float	vec2_length(const vec2 v);
float	vec2_angle_from_to(const vec2 a, const vec2 b);
float	vec2_distance(const vec2 a, const vec2 b);


vec3	vec3_make(const float x, const float y, const float z);
vec3	vec3_zero();
vec3	vec3_ones();
vec3	vec3_max(const vec3 lhs, const vec3 rhs);
vec3	vec3_min(const vec3 lhs, const vec3 rhs);
void	vec3_print(const vec3 v);
vec3	vec3_add(const vec3 lhs, const vec3 rhs);
vec3	vec3_sub(const vec3 lhs, const vec3 rhs);
vec3	vec3_mult(const vec3 lhs, const vec3 rhs);
vec3	vec3_mult_scalar(const vec3 lhs, float rhs);
vec3	vec3_div(const vec3 lhs, const vec3 rhs);
vec3	vec3_div_scalar(const vec3 lhs, float rhs);
vec3	vec3_negate(const vec3 v);
vec3	vec3_cross(const vec3 lhs, const vec3 rhs);
float	vec3_dot(const vec3 lhs, const vec3 rhs);
vec3	vec3_norm(const vec3 v);
float	vec3_length(const vec3 v);
vec3	vec3_lerp(const vec3 min, const vec3 max, float t);
float	vec3_distance(const vec3 a, const vec3 b);
float	vec3_distance_squared(const vec3 a, const vec3 b);


vec4	vec4_make(const float x, const float y, const float z, const float w);
vec4	vec4_make_from_vec3(const vec3 v, const float w);
vec4	vec4_zero();
vec4	vec4_ones();
vec4	vec4_max(const vec4 lhs, const vec4 rhs);
vec4	vec4_min(const vec4 lhs, const vec4 rhs);
float	vec4_dot(const vec4 lhs, const vec4 rhs);
vec4	vec4_lerp(const vec4 min, const vec4 max, float t);
float	vec4_length(const vec4 v);
void	vec4_print(const vec4 v);


quat	quat_make(float x, float y, float z, float w);
float	quat_length(const quat q);
#if 0
float	quat_angle(const quat q);
vec3	quat_axis(const quat q);
#endif
quat	quat_add(const quat lhs, const quat rhs);
quat	quat_sub(const quat lhs, const quat rhs);
quat	quat_mult(const quat lhs, const quat rhs);


float	distancef(float x, float y);
float	parabola(float x, float k);

float	lerp1f(float min, float max, float t); // NOTE(TS): lerp() clashes with OpenAL
float	clamp(float f, float min, float max);
float	randr(float min, float max);

int		imax(int a, int b); // TODO(TS): remove, prefer max()
int		imin(int a, int b); // TODO(TS): remove, prefer min()
int		sign(int x);
int		powi(int base, int exp);
int		log2i(unsigned int val);

#endif
