/**
 * 4 dimensional math functions.
 *
 * Authors: Tim Sjöstrand <tim.sjostrand@gmail.com>
 *			Johan Yngman <johan.yngman@gmailcom>
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>

#include "math4.h"

#define PRINTF_4F "% 8f\t% 8f\t% 8f\t% 8f"
#define PRINTF_3F "% 8f\t% 8f\t% 8f"

void mat4_print(const mat4 m)
{
	printf(PRINTF_4F "\n", m.m[0],  m.m[1],  m.m[2],	m.m[3]);
	printf(PRINTF_4F "\n", m.m[4],  m.m[5],  m.m[6],	m.m[7]);
	printf(PRINTF_4F "\n", m.m[8],  m.m[9],  m.m[10],	m.m[11]);
	printf(PRINTF_4F "\n", m.m[12], m.m[13], m.m[14],	m.m[15]);
}

mat4 mat4_zero()
{
	return (mat4) {
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
	};
}

mat4 mat4_ones()
{
	return (mat4) {
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
	};
}

mat4 mat4_ortho(float left, float right, float bottom, float top, float nearZ, float farZ)
{
	const float ral = right + left;
	const float rsl = right - left;
	const float tab = top + bottom;
	const float tsb = top - bottom;
	const float fan = farZ + nearZ;
	const float fsn = farZ - nearZ;
	
	return (mat4) {
		2.0f / rsl, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / tsb, 0.0f, 0.0f,
		0.0f, 0.0f, -2.0f / fsn, 0.0f,
		-ral / rsl, -tab / tsb, -fan / fsn, 1.0f
	};
#if 0
	mat4 m = {
		2/(r-l),			0.0f,				0.0f,			0.0f,
		0.0f,				2/(t-b),			0.0f,			0.0f,
		0.0f,				0.0f,				(-2)/(f-n),		0.0f,
		-((r+l)/(r-l)),		-((t+b)/(t-b)),		-((f+n)/(f-n)), 1.0f
	};
	return m;
#endif
}

mat4 mat4_frustum(float left, float right, float bottom, float top, float near, float far)
{
	const float ral = right + left;
	const float rsl = right - left;
	const float tsb = top - bottom;
	const float tab = top + bottom;
	const float fan = near + far;
	const float fsn = far - near;
	
	return (mat4) {
		2.0f * near / rsl,		0.0f,				0.0f,							0.0f,
		0.0f,					2.0f * near / tsb,	0.0f,							0.0f,
		ral / rsl,				tab / tsb,			-fan / fsn,						-1.0f,
		0.0f,					0.0f,				(-2.0f * far * near) / fsn,		0.0f
	};
}

mat4 mat4_make( float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	return (mat4) {
		m00, m01, m02, m03,
		m10, m11, m12, m13,
		m20, m21, m22, m23,
		m30, m31, m32, m33
	};
}

mat4 mat4_perspective(float fov_y, float ratio, float near, float far)
{
	const float cotan = 1.0f / tanf(fov_y / 2.0f);
	
	return (mat4) {
		cotan / ratio,	0.0f,		0.0f,									0.0f,
		0.0f,			cotan,		0.0f,									0.0f,
		0.0f,			0.0f,		(far + near) / (near - far),			-1.0f,
		0.0f,			0.0f,		(2.0f * far * near) / (near - far),		0.0f
	};
}

mat4 mat4_lookat(const vec3 eye_pos, const vec3 lookat_pos, const vec3 up)
{
	const vec3 n = vec3_norm(vec3_sub(lookat_pos, eye_pos));
	const vec3 u = vec3_norm(vec3_cross(up, n));
	const vec3 v = vec3_cross(n, u);
	
	return (mat4) {
		u.x, v.x, n.x,				0.0f,
		u.y, v.y, n.y,				0.0f,
		u.z, v.z, n.z,				0.0f,
		-vec3_dot(u, eye_pos),
		-vec3_dot(v, eye_pos),
		-vec3_dot(n, eye_pos),
		1.0f
	};
}

mat4 mat4_look(const vec3 eye_pos, const vec3 dir, const vec3 up)
{
	//
	// FIXME(TS): dir needs to be negated -- fix in mat4_lookat?
	//

	return mat4_lookat(eye_pos, vec3_add(eye_pos, dir), up);
}

mat4 mat4_identity()
{
	return mat4_make_diagonal(1.0f);
}

mat4 mat4_make_diagonal(float s)
{
	return (mat4) {
		s,		0.0f,	0.0f, 	0.0f,
		0.0f,	s,		0.0f, 	0.0f,
		0.0f,	0.0f,	s,		0.0f,
		0.0f,	0.0f,	0.0f, 	s,
	};
}

mat4 mat4_translation(float x, float y, float z)
{
	mat4 m = mat4_identity();
	m.m[12] = x;
	m.m[13] = y;
	m.m[14] = z;
	return m;
}

mat4 mat4_scaling(float sx, float sy, float sz)
{
	mat4 m = mat4_identity();
	m.m[0] = sx;
	m.m[5] = sy;
	m.m[10] = sz;
	return m;
}

mat4 mat4_translate(const mat4 m, float x, float y, float z)
{
	const mat4 translation = mat4_translation(x, y, z);
	return mat4_mult(m, translation);
}

mat4 mat4_scale(const mat4 m, float sx, float sy, float sz)
{
    return (mat4) {
		m.m[0] * sx, m.m[1] * sx, m.m[2] * sx, m.m[3] * sx,
		m.m[4] * sy, m.m[5] * sy, m.m[6] * sy, m.m[7] * sy,
		m.m[8] * sz, m.m[9] * sz, m.m[10] * sz, m.m[11] * sz,
		m.m[12], m.m[13], m.m[14], m.m[15]
	};
}

vec4 mat4_mult_vec4(const mat4 m, const vec4 a)
{
#if 1
	return (vec4) {
		m.m[0]  * a.v[0] + m.m[4]  * a.v[1] + m.m[8]  * a.v[2] + m.m[12]  * a.v[3],
		m.m[1]  * a.v[0] + m.m[5]  * a.v[1] + m.m[9]  * a.v[2] + m.m[13]  * a.v[3],
		m.m[2]  * a.v[0] + m.m[6]  * a.v[1] + m.m[10] * a.v[2] + m.m[14] * a.v[3],
		m.m[3] * a.v[0] + m.m[7] * a.v[1] + m.m[11] * a.v[2] + m.m[15] * a.v[3],
	};
#else
	return (vec4) {
		m.m[0]  * a.v[0] + m.m[1]  * a.v[1] + m.m[2]  * a.v[2] + m.m[3]  * a.v[3],
		m.m[4]  * a.v[0] + m.m[5]  * a.v[1] + m.m[6]  * a.v[2] + m.m[7]  * a.v[3],
		m.m[8]  * a.v[0] + m.m[9]  * a.v[1] + m.m[10] * a.v[2] + m.m[11] * a.v[3],
		m.m[12] * a.v[0] + m.m[13] * a.v[1] + m.m[14] * a.v[2] + m.m[15] * a.v[3],
	};
#endif
}

mat4 mat4_rotation_x(const float radians)
{
	const float cos = cosf(radians);
	const float sin = sinf(radians);
	return (mat4) {
		1.0f,	0.0f,	0.0f,	0.0f,
		0.0f,	cos,	sin,	0.0f,
		0.0f,	-sin,	cos,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};
}

mat4 mat4_rotation_y(const float radians)
{
	const float cos = cosf(radians);
	const float sin = sinf(radians);
	return (mat4) {
		cos,	0.0f,	-sin,	0.0f,
		0.0f,	1.0f,	0.0f,	0.0f,
		sin,	0.0f,	cos,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};
}

mat4 mat4_rotation_z(const float radians)
{
	const float cos = cosf(radians);
	const float sin = sinf(radians);
	return (mat4) {
		cos,	sin,	0.0f,	0.0f,
		-sin,	cos,	0.0f,	0.0f,
		0.0f,	0.0f,	1.0f,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};
}

mat4 mat4_rotate_x(const mat4 m, const float radians)
{
	const mat4 rotation = mat4_rotation_x(radians);
	return mat4_mult(m, rotation);
}

mat4 mat4_rotate_y(const mat4 m, const float radians)
{
	const mat4 rotation = mat4_rotation_y(radians);
	return mat4_mult(m, rotation);
}

mat4 mat4_rotate_z(const mat4 m, const float radians)
{
	const mat4 rotation = mat4_rotation_z(radians);
	return mat4_mult(m, rotation);
}

mat4 mat4_mult(const mat4 lhs, const mat4 rhs)
{
	mat4 m;

	m.m[0]  = lhs.m[0] * rhs.m[0]  + lhs.m[4] * rhs.m[1]  + lhs.m[8] * rhs.m[2]   + lhs.m[12] * rhs.m[3];
	m.m[4]  = lhs.m[0] * rhs.m[4]  + lhs.m[4] * rhs.m[5]  + lhs.m[8] * rhs.m[6]   + lhs.m[12] * rhs.m[7];
	m.m[8]  = lhs.m[0] * rhs.m[8]  + lhs.m[4] * rhs.m[9]  + lhs.m[8] * rhs.m[10]  + lhs.m[12] * rhs.m[11];
	m.m[12] = lhs.m[0] * rhs.m[12] + lhs.m[4] * rhs.m[13] + lhs.m[8] * rhs.m[14]  + lhs.m[12] * rhs.m[15];

	m.m[1]  = lhs.m[1] * rhs.m[0]  + lhs.m[5] * rhs.m[1]  + lhs.m[9] * rhs.m[2]   + lhs.m[13] * rhs.m[3];
	m.m[5]  = lhs.m[1] * rhs.m[4]  + lhs.m[5] * rhs.m[5]  + lhs.m[9] * rhs.m[6]   + lhs.m[13] * rhs.m[7];
	m.m[9]  = lhs.m[1] * rhs.m[8]  + lhs.m[5] * rhs.m[9]  + lhs.m[9] * rhs.m[10]  + lhs.m[13] * rhs.m[11];
	m.m[13] = lhs.m[1] * rhs.m[12] + lhs.m[5] * rhs.m[13] + lhs.m[9] * rhs.m[14]  + lhs.m[13] * rhs.m[15];

	m.m[2]  = lhs.m[2] * rhs.m[0]  + lhs.m[6] * rhs.m[1]  + lhs.m[10] * rhs.m[2]  + lhs.m[14] * rhs.m[3];
	m.m[6]  = lhs.m[2] * rhs.m[4]  + lhs.m[6] * rhs.m[5]  + lhs.m[10] * rhs.m[6]  + lhs.m[14] * rhs.m[7];
	m.m[10] = lhs.m[2] * rhs.m[8]  + lhs.m[6] * rhs.m[9]  + lhs.m[10] * rhs.m[10] + lhs.m[14] * rhs.m[11];
	m.m[14] = lhs.m[2] * rhs.m[12] + lhs.m[6] * rhs.m[13] + lhs.m[10] * rhs.m[14] + lhs.m[14] * rhs.m[15];

	m.m[3]  = lhs.m[3] * rhs.m[0]  + lhs.m[7] * rhs.m[1]  + lhs.m[11] * rhs.m[2]  + lhs.m[15] * rhs.m[3];
	m.m[7]  = lhs.m[3] * rhs.m[4]  + lhs.m[7] * rhs.m[5]  + lhs.m[11] * rhs.m[6]  + lhs.m[15] * rhs.m[7];
	m.m[11] = lhs.m[3] * rhs.m[8]  + lhs.m[7] * rhs.m[9]  + lhs.m[11] * rhs.m[10] + lhs.m[15] * rhs.m[11];
	m.m[15] = lhs.m[3] * rhs.m[12] + lhs.m[7] * rhs.m[13] + lhs.m[11] * rhs.m[14] + lhs.m[15] * rhs.m[15];

	return m;
}

mat4 mat4_transpose(const mat4 m)
{
	return (mat4) {
		m.m[ 0],	m.m[ 4],	m.m[ 8],	m.m[12],
		m.m[ 1],	m.m[ 5],	m.m[ 9],	m.m[13],
		m.m[ 2],	m.m[ 6],	m.m[10],	m.m[14],
		m.m[ 3],	m.m[ 7],	m.m[11],	m.m[15],
	};
}

/**
 * Transposes the matrix 'a'.
 *
 * Assuming an NxN matrix 'A':
 * for n = 0 to N - 2
 *	for m = n + 1 to N - 1
 *	 swap A(n,m) with A(m,n)
 */
void mat4_transpose_same(mat4 *m)
{
	float tmp;
	// n=0, m=[1,3]
	swapf(tmp, m->m[0*4 + 1], m->m[1*4 + 0]);
	swapf(tmp, m->m[0*4 + 2], m->m[2*4 + 0]);
	swapf(tmp, m->m[0*4 + 3], m->m[3*4 + 0]);
	// n=1, m=[2,3]
	swapf(tmp, m->m[1*4 + 2], m->m[2*4 + 1]);
	swapf(tmp, m->m[1*4 + 3], m->m[3*4 + 1]);
	// n=2, m=[3,3]
	swapf(tmp, m->m[2*4 + 3], m->m[3*4 + 2]);
}

vec3 vec3_cross(const vec3 lhs, const vec3 rhs)
{
	return (vec3) {
		lhs.v[1] * rhs.v[2] - lhs.v[2] * rhs.v[1],
		lhs.v[2] * rhs.v[0] - lhs.v[0] * rhs.v[2],
		lhs.v[0] * rhs.v[1] - lhs.v[1] * rhs.v[0]
	};
}

float vec3_dot(const vec3 lhs, const vec3 rhs)
{
	return lhs.v[0] * rhs.v[0] + lhs.v[1] * rhs.v[1] + lhs.v[2] * rhs.v[2];
}

vec3 vec3_add(const vec3 left, const vec3 right)
{
	return (vec3) {
		left.v[0] + right.v[0],
		left.v[1] + right.v[1],
		left.v[2] + right.v[2],
	};
}

vec3 vec3_sub(const vec3 left, const vec3 right)
{
	return (vec3) {
		left.v[0] - right.v[0],
		left.v[1] - right.v[1],
		left.v[2] - right.v[2],
	};
}

vec3 vec3_mult(const vec3 left, const vec3 right)
{
	return (vec3) {
		left.v[0] * right.v[0],
		left.v[1] * right.v[1],
		left.v[2] * right.v[2],
	};
}

vec3 vec3_mult_scalar(const vec3 lhs, float rhs)
{
	return (vec3) {
		lhs.v[0] * rhs,
		lhs.v[1] * rhs,
		lhs.v[2] * rhs
	};
}

vec3 vec3_div(const vec3 left, const vec3 right)
{
	return (vec3) {
		left.v[0] / right.v[0],
		left.v[1] / right.v[1],
		left.v[2] / right.v[2],
	};
}

vec3 vec3_div_scalar(const vec3 lhs, float rhs)
{
	return (vec3) {
		lhs.v[0] / rhs,
		lhs.v[1] / rhs,
		lhs.v[2] / rhs
	};
}

vec3 vec3_negate(const vec3 v)
{
	return (vec3) {
		-v.v[0],
		-v.v[1],
		-v.v[2],
	};
}

mat4 mat4_adjugate(const mat4 a)
{
	return (mat4) {
		-a.m[ 7]*a.m[10]*a.m[13]+a.m[ 6]*a.m[11]*a.m[13]+a.m[ 7]*a.m[ 9]*a.m[14]-a.m[ 5]*a.m[11]*a.m[14]-a.m[ 6]*a.m[ 9]*a.m[15]+a.m[ 5]*a.m[10]*a.m[15],
		 a.m[ 3]*a.m[10]*a.m[13]-a.m[ 2]*a.m[11]*a.m[13]-a.m[ 3]*a.m[ 9]*a.m[14]+a.m[ 1]*a.m[11]*a.m[14]+a.m[ 2]*a.m[ 9]*a.m[15]-a.m[ 1]*a.m[10]*a.m[15],
		-a.m[ 3]*a.m[ 6]*a.m[13]+a.m[ 2]*a.m[ 7]*a.m[13]+a.m[ 3]*a.m[ 5]*a.m[14]-a.m[ 1]*a.m[ 7]*a.m[14]-a.m[ 2]*a.m[ 5]*a.m[15]+a.m[ 1]*a.m[ 6]*a.m[15],
		 a.m[ 3]*a.m[ 6]*a.m[ 9]-a.m[ 2]*a.m[ 7]*a.m[ 9]-a.m[ 3]*a.m[ 5]*a.m[10]+a.m[ 1]*a.m[ 7]*a.m[10]+a.m[ 2]*a.m[ 5]*a.m[11]-a.m[ 1]*a.m[ 6]*a.m[11],
		 a.m[ 7]*a.m[10]*a.m[12]-a.m[ 6]*a.m[11]*a.m[12]-a.m[ 7]*a.m[ 8]*a.m[14]+a.m[ 4]*a.m[11]*a.m[14]+a.m[ 6]*a.m[ 8]*a.m[15]-a.m[ 4]*a.m[10]*a.m[15],
		-a.m[ 3]*a.m[10]*a.m[12]+a.m[ 2]*a.m[11]*a.m[12]+a.m[ 3]*a.m[ 8]*a.m[14]-a.m[ 0]*a.m[11]*a.m[14]-a.m[ 2]*a.m[ 8]*a.m[15]+a.m[ 0]*a.m[10]*a.m[15],
		 a.m[ 3]*a.m[ 6]*a.m[12]-a.m[ 2]*a.m[ 7]*a.m[12]-a.m[ 3]*a.m[ 4]*a.m[14]+a.m[ 0]*a.m[ 7]*a.m[14]+a.m[ 2]*a.m[ 4]*a.m[15]-a.m[ 0]*a.m[ 6]*a.m[15],
		-a.m[ 3]*a.m[ 6]*a.m[ 8]+a.m[ 2]*a.m[ 7]*a.m[ 8]+a.m[ 3]*a.m[ 4]*a.m[10]-a.m[ 0]*a.m[ 7]*a.m[10]-a.m[ 2]*a.m[ 4]*a.m[11]+a.m[ 0]*a.m[ 6]*a.m[11],
		-a.m[ 7]*a.m[ 9]*a.m[12]+a.m[ 5]*a.m[11]*a.m[12]+a.m[ 7]*a.m[ 8]*a.m[13]-a.m[ 4]*a.m[11]*a.m[13]-a.m[ 5]*a.m[ 8]*a.m[15]+a.m[ 4]*a.m[ 9]*a.m[15],
		 a.m[ 3]*a.m[ 9]*a.m[12]-a.m[ 1]*a.m[11]*a.m[12]-a.m[ 3]*a.m[ 8]*a.m[13]+a.m[ 0]*a.m[11]*a.m[13]+a.m[ 1]*a.m[ 8]*a.m[15]-a.m[ 0]*a.m[ 9]*a.m[15],
		-a.m[ 3]*a.m[ 5]*a.m[12]+a.m[ 1]*a.m[ 7]*a.m[12]+a.m[ 3]*a.m[ 4]*a.m[13]-a.m[ 0]*a.m[ 7]*a.m[13]-a.m[ 1]*a.m[ 4]*a.m[15]+a.m[ 0]*a.m[ 5]*a.m[15],
		 a.m[ 3]*a.m[ 5]*a.m[ 8]-a.m[ 1]*a.m[ 7]*a.m[ 8]-a.m[ 3]*a.m[ 4]*a.m[ 9]+a.m[ 0]*a.m[ 7]*a.m[ 9]+a.m[ 1]*a.m[ 4]*a.m[11]-a.m[ 0]*a.m[ 5]*a.m[11],
		 a.m[ 6]*a.m[ 9]*a.m[12]-a.m[ 5]*a.m[10]*a.m[12]-a.m[ 6]*a.m[ 8]*a.m[13]+a.m[ 4]*a.m[10]*a.m[13]+a.m[ 5]*a.m[ 8]*a.m[14]-a.m[ 4]*a.m[ 9]*a.m[14],
		-a.m[ 2]*a.m[ 9]*a.m[12]+a.m[ 1]*a.m[10]*a.m[12]+a.m[ 2]*a.m[ 8]*a.m[13]-a.m[ 0]*a.m[10]*a.m[13]-a.m[ 1]*a.m[ 8]*a.m[14]+a.m[ 0]*a.m[ 9]*a.m[14],
		 a.m[ 2]*a.m[ 5]*a.m[12]-a.m[ 1]*a.m[ 6]*a.m[12]-a.m[ 2]*a.m[ 4]*a.m[13]+a.m[ 0]*a.m[ 6]*a.m[13]+a.m[ 1]*a.m[ 4]*a.m[14]-a.m[ 0]*a.m[ 5]*a.m[14],
		-a.m[ 2]*a.m[ 5]*a.m[ 8]+a.m[ 1]*a.m[ 6]*a.m[ 8]+a.m[ 2]*a.m[ 4]*a.m[ 9]-a.m[ 0]*a.m[ 6]*a.m[ 9]-a.m[ 1]*a.m[ 4]*a.m[10]+a.m[ 0]*a.m[ 5]*a.m[10],
	};
}

float mat4_determinant(const mat4 m)
{
	return +m.m[ 3]*m.m[ 6]*m.m[ 9]*m.m[12]
		   -m.m[ 2]*m.m[ 7]*m.m[ 9]*m.m[12]
		   -m.m[ 3]*m.m[ 5]*m.m[10]*m.m[12]
		   +m.m[ 1]*m.m[ 7]*m.m[10]*m.m[12]
		   +m.m[ 2]*m.m[ 5]*m.m[11]*m.m[12]
		   -m.m[ 1]*m.m[ 6]*m.m[11]*m.m[12]
		   -m.m[ 3]*m.m[ 6]*m.m[ 8]*m.m[13]
		   +m.m[ 2]*m.m[ 7]*m.m[ 8]*m.m[13]
		   +m.m[ 3]*m.m[ 4]*m.m[10]*m.m[13]
		   -m.m[ 0]*m.m[ 7]*m.m[10]*m.m[13]
		   -m.m[ 2]*m.m[ 4]*m.m[11]*m.m[13]
		   +m.m[ 0]*m.m[ 6]*m.m[11]*m.m[13]
		   +m.m[ 3]*m.m[ 5]*m.m[ 8]*m.m[14]
		   -m.m[ 1]*m.m[ 7]*m.m[ 8]*m.m[14]
		   -m.m[ 3]*m.m[ 4]*m.m[ 9]*m.m[14]
		   +m.m[ 0]*m.m[ 7]*m.m[ 9]*m.m[14]
		   +m.m[ 1]*m.m[ 4]*m.m[11]*m.m[14]
		   -m.m[ 0]*m.m[ 5]*m.m[11]*m.m[14]
		   -m.m[ 2]*m.m[ 5]*m.m[ 8]*m.m[15]
		   +m.m[ 1]*m.m[ 6]*m.m[ 8]*m.m[15]
		   +m.m[ 2]*m.m[ 4]*m.m[ 9]*m.m[15]
		   -m.m[ 0]*m.m[ 6]*m.m[ 9]*m.m[15]
		   -m.m[ 1]*m.m[ 4]*m.m[10]*m.m[15]
		   +m.m[ 0]*m.m[ 5]*m.m[10]*m.m[15];
}

mat4 mat4_mult_scalar(const mat4 m, const float s)
{
	return (mat4) {
		 m.m[ 0]*s,	m.m[ 1]*s,	m.m[ 2]*s,	m.m[ 3]*s,
		 m.m[ 4]*s,	m.m[ 5]*s,	m.m[ 6]*s,	m.m[ 7]*s,
		 m.m[ 8]*s,	m.m[ 9]*s,	m.m[10]*s,	m.m[11]*s,
		 m.m[12]*s,	m.m[13]*s,	m.m[14]*s,	m.m[15]*s,
	};
}

mat4 mat4_add(const mat4 lhs, const mat4 rhs)
{
	return (mat4) {
		lhs.m[ 0] + rhs.m[ 0], lhs.m[ 1] + rhs.m[ 1], lhs.m[ 2] + rhs.m[ 2], lhs.m[ 3] + rhs.m[ 3],
		lhs.m[ 4] + rhs.m[ 4], lhs.m[ 5] + rhs.m[ 5], lhs.m[ 6] + rhs.m[ 6], lhs.m[ 7] + rhs.m[ 7],
		lhs.m[ 8] + rhs.m[ 8], lhs.m[ 9] + rhs.m[ 9], lhs.m[10] + rhs.m[10], lhs.m[11] + rhs.m[11],
		lhs.m[12] + rhs.m[12], lhs.m[13] + rhs.m[13], lhs.m[14] + rhs.m[14], lhs.m[15] + rhs.m[15],
	};
}

mat4 mat4_inverse(const mat4 matrix, int* is_invertable)
{
	// m = transposed cofactor matrix
	float c02070306 = matrix.m[2] * matrix.m[7] - matrix.m[3] * matrix.m[6];
	float c02110310 = matrix.m[2] * matrix.m[11] - matrix.m[3] * matrix.m[10];
	float c02150314 = matrix.m[2] * matrix.m[15] - matrix.m[3] * matrix.m[14];
	float c06110710 = matrix.m[6] * matrix.m[11] - matrix.m[7] * matrix.m[10];
	float c06150714 = matrix.m[6] * matrix.m[15] - matrix.m[7] * matrix.m[14];
	float c10151114 = matrix.m[10] * matrix.m[15] - matrix.m[11] * matrix.m[14];
	mat4 m = {
		matrix.m[5] * c10151114 + matrix.m[9] * -c06150714 + matrix.m[13] * c06110710, // c0
		-matrix.m[1] * c10151114 + matrix.m[9] * c02150314 - matrix.m[13] * c02110310, // c4
		matrix.m[1] * c06150714 - matrix.m[5] * c02150314 + matrix.m[13] * c02070306, // c8
		-matrix.m[1] * c06110710 + matrix.m[5] * c02110310 - matrix.m[9] * c02070306, // c12
		0.0f, // c1
		0.0f, // c5
		0.0f, // c9
		0.0f, // c13
		0.0f, // c2
		0.0f, // c6
		0.0f, // c10
		0.0f, // c14
		0.0f, // c3
		0.0f, // c7
		0.0f, // c11
		0.0f, // c15
	};
	// d = matrix determinant
	float d = m.m[0] * matrix.m[0] + m.m[1] * matrix.m[4] + m.m[2] * matrix.m[8] + m.m[3] * matrix.m[12];
	if (fabsf(d) < FLT_EPSILON) {
		if(is_invertable) {
			*is_invertable = 0;
		}
		return mat4_identity();
	}
	if(is_invertable) {
		*is_invertable = 1;
	}
	// d = 1 / matrix determinant
	d = 1.0f / d;
	// m = transposed inverse matrix = transposed cofactor matrix * d
	float c01070305 = matrix.m[1] * matrix.m[7] - matrix.m[3] * matrix.m[5];
	float c01110309 = matrix.m[1] * matrix.m[11] - matrix.m[3] * matrix.m[9];
	float c01150313 = matrix.m[1] * matrix.m[15] - matrix.m[3] * matrix.m[13];
	float c05110709 = matrix.m[5] * matrix.m[11] - matrix.m[7] * matrix.m[9];
	float c05150713 = matrix.m[5] * matrix.m[15] - matrix.m[7] * matrix.m[13];
	float c09151113 = matrix.m[9] * matrix.m[15] - matrix.m[11] * matrix.m[13];

	float c01060205 = matrix.m[1] * matrix.m[6] - matrix.m[2] * matrix.m[5];
	float c01100209 = matrix.m[1] * matrix.m[10] - matrix.m[2] * matrix.m[9];
	float c01140213 = matrix.m[1] * matrix.m[14] - matrix.m[2] * matrix.m[13];
	float c05100609 = matrix.m[5] * matrix.m[10] - matrix.m[6] * matrix.m[9];
	float c05140613 = matrix.m[5] * matrix.m[14] - matrix.m[6] * matrix.m[13];
	float c09141013 = matrix.m[9] * matrix.m[14] - matrix.m[10] * matrix.m[13];

	m.m[0] *= d; // c0
	m.m[1] *= d, // c4
	m.m[2] *= d; // c8
	m.m[3] *= d; // c12
	m.m[4] = (-matrix.m[4] * c10151114 + matrix.m[8] * c06150714 - matrix.m[12] * c06110710) * d; // c1
	m.m[5] = (matrix.m[0] * c10151114 - matrix.m[8] * c02150314 + matrix.m[12] * c02110310) * d; // c5
	m.m[6] = (-matrix.m[0] * c06150714 + matrix.m[4] * c02150314 - matrix.m[12] * c02070306) * d; // c9
	m.m[7] = (matrix.m[0] * c06110710 - matrix.m[4] * c02110310 + matrix.m[8] * c02070306) * d; // c13
	m.m[8] = (matrix.m[4] * c09151113 - matrix.m[8] * c05150713 + matrix.m[12] * c05110709) * d; // c2
	m.m[9] = (-matrix.m[0] * c09151113 + matrix.m[8] * c01150313 - matrix.m[12] * c01110309) * d; // c6
	m.m[10] = (matrix.m[0] * c05150713 - matrix.m[4] * c01150313 + matrix.m[12] * c01070305) * d; // c10
	m.m[11] = (-matrix.m[0] * c05110709 + matrix.m[4] * c01110309 - matrix.m[8] * c01070305) * d; // c14
	m.m[12] = (-matrix.m[4] * c09141013 + matrix.m[8] * c05140613 - matrix.m[12] * c05100609) * d; // c3
	m.m[13] = (matrix.m[0] * c09141013 - matrix.m[8] * c01140213 + matrix.m[12] * c01100209) * d; // c7
	m.m[14] = (-matrix.m[0] * c05140613 + matrix.m[4] * c01140213 - matrix.m[12] * c01060205) * d; // c11
	m.m[15] = (matrix.m[0] * c05100609 - matrix.m[4] * c01100209 + matrix.m[8] * c01060205) * d; // c15
	return m;
}


vec2 vec2_make(const float x, const float y)
{
	return (vec2) { x, y };
}

vec2 vec2_lerp(const vec2 min, const vec2 max, float t)
{
	return (vec2) {
		lerp1f(min.x, max.y, t),
		lerp1f(min.y, max.y, t)
	};
}

float vec2_distance(const vec2 a, const vec2 b)
{
	return distancef(b.v[0]-a.v[0], b.v[1]-a.v[1]);
}

vec2 vec2_norm(const vec2 v)
{
	const float length = vec2_length(v);

	// FIXME(TS): use float_is_nearly_equal()
	if (length == 0.0f)
		return v;

	return (vec2) {
		v.v[0] / length,
		v.v[1] / length,
	};
}

float vec2_length(const vec2 v)
{
	return sqrtf(v.v[0]*v.v[0] + v.v[1]*v.v[1]);
}

float vec2_angle_from_to(const vec2 a, const vec2 b)
{
	return atan2f(a.v[0]-b.v[0], a.v[1]-b.v[1]);
}

vec2 vec2_add(const vec2 lhs, const vec2 rhs)
{
	return (vec2) {
		lhs.v[0] + rhs.v[0],
		lhs.v[0] + rhs.v[0]
	};
}

vec2 vec2_sub(const vec2 lhs, const vec2 rhs)
{
	return (vec2) {
		lhs.v[0] - rhs.v[0],
		lhs.v[0] - rhs.v[0]
	};
}

vec2 vec2_mult(const vec2 lhs, const vec2 rhs)
{
	return (vec2) {
		lhs.v[0] * rhs.v[0],
		lhs.v[0] * rhs.v[0]
	};
}


vec3 vec3_make(const float x, const float y, const float z)
{
	return (vec3) { x, y, z };
}

vec3 vec3_zero()
{
	return vec3_make(0.0f, 0.0f, 0.0f);
}

vec3 vec3_ones()
{
	return vec3_make(1.0f, 1.0f, 1.0f);
}

vec3 vec3_max(const vec3 lhs, const vec3 rhs)
{
	return (vec3) {
		.x = max(lhs.x, rhs.x),
		.y = max(lhs.y, rhs.y),
		.z = max(lhs.z, rhs.z),
	};
}

vec3 vec3_min(const vec3 lhs, const vec3 rhs)
{
	return (vec3) {
		.x = min(lhs.x, rhs.x),
		.y = min(lhs.y, rhs.y),
		.z = min(lhs.z, rhs.z),
	};
}

void vec3_print(const vec3 v)
{
	printf(PRINTF_3F "\n", v.v[0], v.v[1], v.v[2]);
}

vec3 vec3_norm(const vec3 v)
{
	const float length = vec3_length(v);

	// FIXME(TS): use float_is_nearly_equal()
	if (length == 0.0f)
		return v;

	return (vec3) {
		v.v[0] / length,
		v.v[1] / length,
		v.v[2] / length,
	};
}

vec3 vec3_add3f(const vec3 v, const float x, const float y, const float z)
{
	return (vec3) {
		v.v[0] + x,
		v.v[1] + y,
		v.v[2] + z,
	};
}

vec3 vec3_sub3f(const vec3 v, const float x, const float y, const float z)
{
	return (vec3) {
		v.v[0] - x,
		v.v[1] - y,
		v.v[2] - z,
	};
}

vec3 vec3_mult3f(const vec3 v, const float x, const float y, const float z)
{
	return (vec3) {
		v.v[0] * x,
		v.v[1] * y,
		v.v[2] * z,
	};
}

vec3 vec3_lerp(const vec3 min, const vec3 max, float t)
{
	return (vec3) {
		lerp1f(min.v[0], max.v[0], t),
		lerp1f(min.v[1], max.v[1], t),
		lerp1f(min.v[2], max.v[2], t)
	};
}

float vec3_distance_squared(const vec3 a, const vec3 b)
{
	const float xd = a.v[0] - b.v[0];
	const float yd = a.v[1] - b.v[1];
	const float zd = a.v[2] - b.v[2];
	return xd*xd + yd*yd + zd*zd;
}

float vec3_distance(const vec3 a, const vec3 b)
{
	return sqrtf(vec3_distance_squared(a, b));
}

float vec3_length(const vec3 v)
{
	return sqrtf(
		v.v[0]*v.v[0] +
		v.v[1]*v.v[1] +
		v.v[2]*v.v[2]
	);
}


vec4 vec4_make(const float x, const float y, const float z, const float w)
{
	return (vec4) { x, y, z, w };
}

vec4 vec4_make_from_vec3(const vec3 v, const float w)
{
	return (vec4) {
		.x = v.x,
		.y = v.y,
		.z = v.z,
		.w = w
	};
}

vec4 vec4_zero()
{
	return vec4_make(0.0f, 0.0f, 0.0f, 0.0f);
}

vec4 vec4_ones()
{
	return vec4_make(1.0f, 1.0f, 1.0f, 1.0f);
}

vec4 vec4_max(const vec4 lhs, const vec4 rhs)
{
	return (vec4) {
		.x = max(lhs.x, rhs.x),
		.y = max(lhs.y, rhs.y),
		.z = max(lhs.z, rhs.z),
		.w = max(lhs.w, rhs.w),
	};
}

vec4 vec4_min(const vec4 lhs, const vec4 rhs)
{
	return (vec4) {
		.x = min(lhs.x, rhs.x),
		.y = min(lhs.y, rhs.y),
		.z = min(lhs.z, rhs.z),
		.w = min(lhs.w, rhs.w),
	};
}

vec4 vec4_lerp(const vec4 min, const vec4 max, float t)
{
	return (vec4) {
		lerp1f(min.v[0], max.v[0], t),
		lerp1f(min.v[1], max.v[1], t),
		lerp1f(min.v[2], max.v[2], t),
		lerp1f(min.v[3], max.v[3], t)
	};
}

float vec4_length(const vec4 v)
{
	return sqrtf(
		v.v[0]*v.v[0] +
		v.v[1]*v.v[1] +
		v.v[2]*v.v[2] +
		v.v[3]*v.v[3]
	);
}

void vec4_print(const vec4 v)
{
	printf(PRINTF_4F "\n", v.v[0], v.v[1], v.v[2], v.v[3]);
}


quat quat_make(float x, float y, float z, float w)
{
	return (quat) { x, y, z, w };
}

float quat_length(const quat q)
{
	return sqrtf(
		q.q[0] * q.q[0] +
		q.q[1] * q.q[1] +
		q.q[2] * q.q[2] +
		q.q[3] * q.q[3]
	);
}

#if 0
float quat_angle(const quat q)
{
}

vec3 quat_axis(const quat q)
{
}
#endif

quat quat_add(const quat lhs, const quat rhs)
{
	return (quat) {
		lhs.q[0] + rhs.q[0],
		lhs.q[1] + rhs.q[1],
		lhs.q[2] + rhs.q[2],
		lhs.q[3] + rhs.q[3]
	};
}

quat quat_sub(const quat lhs, const quat rhs)
{
	return (quat) {
		lhs.q[0] - rhs.q[0],
		lhs.q[1] - rhs.q[1],
		lhs.q[2] - rhs.q[2],
		lhs.q[3] - rhs.q[3]
	};
}

quat quat_mult(const quat lhs, const quat rhs)
{
	return (quat) {
		lhs.q[3] * rhs.q[0] +
		lhs.q[0] * rhs.q[3] +
		lhs.q[1] * rhs.q[2] -
		lhs.q[2] * rhs.q[1],

		lhs.q[3] * rhs.q[1] +
		lhs.q[1] * rhs.q[3] +
		lhs.q[2] * rhs.q[0] -
		lhs.q[0] * rhs.q[2],

		lhs.q[3] * rhs.q[2] +
		lhs.q[2] * rhs.q[3] +
		lhs.q[0] * rhs.q[1] -
		lhs.q[1] * rhs.q[0],

		lhs.q[3] * rhs.q[3] -
		lhs.q[0] * rhs.q[0] -
		lhs.q[1] * rhs.q[1] -
		lhs.q[2] * rhs.q[2]
	};
}

quat quat_normalize(const quat q)
{
	const float scale = 1.0f / quat_length(q);
	return (quat) {
		q.q[0] * scale,
		q.q[1] * scale,
		q.q[2] * scale,
		q.q[3] * scale
	};
}


int sign(int x)
{
	return (x > 0) - (x < 0);
}

int powi(int base, int exp)
{
	int result = 1;
	while (exp) {
		if(exp & 1) {
			result *= base;
		}
		exp /= 2;
		base *= base;
	}
	return result;
}

int log2i(unsigned int val)
{
	if (val == 0) return UINT_MAX;
	if (val == 1) return 0;
	unsigned int ret = 0;
	while (val > 1) {
		val >>= 1;
		ret++;
	}
	return ret;
}

int imax(int a, int b)
{
	return a >= b ? a : b;
}

int imin(int a, int b)
{
	return a <= b ? a : b;
}

float clamp(float f, float min, float max)
{
	return f < min ? min : (f > max ? max : f);
}

float randr(float min, float max)
{
	return min + (((float) rand()) / (float) RAND_MAX) * (max - min);
}

float lerp1f(float min, float max, float t)
{
	return (1.0f - t) * min + t * max;
}

float distancef(float x, float y)
{
	return sqrtf((x*x) + (y*y));
}

float parabola(float x, float k)
{
	return powf(4.0f * x * (1.0f - x), k);
}
