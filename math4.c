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

#include "math4.h"

#define PRINTF_4F "% 8f\t% 8f\t% 8f\t% 8f"

void printm(mat4 m)
{
	printf(PRINTF_4F "\n", m[0],  m[1],  m[2],	m[3]);
	printf(PRINTF_4F "\n", m[4],  m[5],  m[6],	m[7]);
	printf(PRINTF_4F "\n", m[8],  m[9],  m[10], m[11]);
	printf(PRINTF_4F "\n", m[12], m[13], m[14], m[15]);
}

void printv(vec4 v)
{
	printf(PRINTF_4F "\n", v[0], v[1], v[2], v[3]);
}

/**
 *
 * param m
 * param l Left
 * param r Right
 * param t Top
 * param b Bottom
 * param n Near
 * param f Far
 */
void ortho(mat4 m, float l, float r, float t, float b, float n, float f)
{
	m[0]  = 2/(r-l); m[1]  =	   0; m[2]	=		   0; m[3]	= -((r+l)/(r-l));
	m[4]  =		  0; m[5]  = 2/(t-b); m[6]	=		   0; m[7]	= -((t+b)/(t-b));
	m[8]  =		  0; m[9]  =	   0; m[10] = (-2)/(f-n); m[11] = -((f+n)/(f-n));
	m[12] =		  0; m[13] =	   0; m[14] =		   0; m[15] =			   1;
}

void identity(mat4 m)
{
	m[0]  = 1; m[1]  = 0; m[2]	= 0; m[3]  = 0;
	m[4]  = 0; m[5]  = 1; m[6]	= 0; m[7]  = 0;
	m[8]  = 0; m[9]  = 0; m[10] = 1; m[11] = 0;
	m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}

void translate(mat4 m, float x, float y, float z)
{
	m[0]  = 1; m[1]  = 0; m[2]	= 0; m[3]  = x;
	m[4]  = 0; m[5]  = 1; m[6]	= 0; m[7]  = y;
	m[8]  = 0; m[9]  = 0; m[10] = 1; m[11] = z;
	m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}

void translatev(mat4 m, vec4 v)
{
	translate(m, xyz(v));
}

void scale(mat4 m, float x, float y, float z)
{
	m[0]  = x; m[1]  = 0; m[2]	= 0; m[3]  = 0;
	m[4]  = 0; m[5]  = y; m[6]	= 0; m[7]  = 0;
	m[8]  = 0; m[9]  = 0; m[10] = z; m[11] = 0;
	m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}

/**
 * Rotate along X axis.
 *
 * param m
 * param a Angle in radians.
 */
void rotate_x(mat4 m, const float a)
{
	m[ 0] = 1; m[ 1] =		 0; m[ 2] =		  0; m[ 3] = 0;
	m[ 4] = 0; m[ 5] =	cos(a); m[ 6] = -sin(a); m[ 7] = 0;
	m[ 8] = 0; m[ 9] =	sin(a); m[10] =  cos(a); m[11] = 0;
	m[12] = 0; m[13] =		 0; m[14] =		  0; m[15] = 1;
}

/**
 * Rotate along Y axis.
 *
 * param m
 * param a Angle in radians.
 */
void rotate_y(mat4 m, const float a)
{
	m[ 0] =  cos(a); m[ 1] = 0; m[ 2] = sin(a); m[ 3] = 0;
	m[ 4] =		  0; m[ 5] = 1; m[ 6] =		 0; m[ 7] = 0;
	m[ 8] = -sin(a); m[ 9] = 0; m[10] = cos(a); m[11] = 0;
	m[12] =		  0; m[13] = 0; m[14] =		 0; m[15] = 1;
}

/**
 * Rotate along Z axis.
 *
 * param m
 * param a Angle in radians.
 */
void rotate_z(mat4 m, const float a)
{
	m[ 0] = cos(a); m[ 1] = -sin(a); m[ 2] = 0; m[ 3] = 0;
	m[ 4] = sin(a); m[ 5] =  cos(a); m[ 6] = 0; m[ 7] = 0;
	m[ 8] =		 0; m[ 9] =		  0; m[10] = 1; m[11] = 0;
	m[12] =		 0; m[13] =		  0; m[14] = 0; m[15] = 1;
}

void scalev(mat4 m, vec4 v)
{
	scale(m, xyz(v));
}

/**
 * A0	B1	 C2   D3
 * E4	F5	 G6   H7
 * I8	J9	 K10  L11
 * M12	N13  O14  P15
 *
 * param m Output
 * param a Input
 * param b Input
 */
void mult(mat4 m, const mat4 a, const mat4 b)
{
	// Column 0
	m[0]  = a[ 0]*b[ 0] + a[ 1]*b[ 4] + a[ 2]*b[ 8] + a[ 3]*b[12];
	m[4]  = a[ 4]*b[ 0] + a[ 5]*b[ 4] + a[ 6]*b[ 8] + a[ 7]*b[12];
	m[8]  = a[ 8]*b[ 0] + a[ 9]*b[ 4] + a[10]*b[ 8] + a[11]*b[12];
	m[12] = a[12]*b[ 0] + a[13]*b[ 4] + a[14]*b[ 8] + a[15]*b[12];
	// Column 1
	m[1]  = a[ 0]*b[ 1] + a[ 1]*b[ 5] + a[ 2]*b[ 9] + a[ 3]*b[13];
	m[5]  = a[ 4]*b[ 1] + a[ 5]*b[ 5] + a[ 6]*b[ 9] + a[ 7]*b[13];
	m[9]  = a[ 8]*b[ 1] + a[ 9]*b[ 5] + a[10]*b[ 9] + a[11]*b[13];
	m[13] = a[12]*b[ 1] + a[13]*b[ 5] + a[14]*b[ 9] + a[15]*b[13];
	// Column 2
	m[2]  = a[ 0]*b[ 2] + a[ 1]*b[ 6] + a[ 2]*b[10] + a[ 3]*b[14];
	m[6]  = a[ 4]*b[ 2] + a[ 5]*b[ 6] + a[ 6]*b[10] + a[ 7]*b[14];
	m[10] = a[ 8]*b[ 2] + a[ 9]*b[ 6] + a[10]*b[10] + a[11]*b[14];
	m[14] = a[12]*b[ 2] + a[13]*b[ 6] + a[14]*b[10] + a[15]*b[14];
	// Column 3
	m[3]  = a[ 0]*b[ 3] + a[ 1]*b[ 7] + a[ 2]*b[11] + a[ 3]*b[15];
	m[7]  = a[ 4]*b[ 3] + a[ 5]*b[ 7] + a[ 6]*b[11] + a[ 7]*b[15];
	m[11] = a[ 8]*b[ 3] + a[ 9]*b[ 7] + a[10]*b[11] + a[11]*b[15];
	m[15] = a[12]*b[ 3] + a[13]*b[ 7] + a[14]*b[11] + a[15]*b[15];
}

void mult_same(mat4 a, const mat4 b)
{
	mult(a, a, b);
}

/**
 * Store the transpose of matrix 'a' in 'm'.
 */
void transpose(mat4 m, mat4 a)
{
	/* Row 0 */
	m[0]  = a[ 0]; m[1]  = a[ 4]; m[2]	= a[ 8]; m[3]  = a[12];
	/* Row 1 */
	m[4]  = a[ 1]; m[5]  = a[ 5]; m[6]	= a[ 9]; m[7]  = a[13];
	/* Row 2 */
	m[8]  = a[ 2]; m[9]  = a[ 6]; m[10] = a[10]; m[11] = a[14];
	/* Row 3 */
	m[12] = a[ 3]; m[13] = a[ 7]; m[14] = a[11]; m[15] = a[15];
}

/**
 * Transposes the matrix 'm'.
 */
void transpose_same_copy(mat4 m)
{
	mat4 tmp;
	transpose(tmp, m);
	copym(m, tmp);
}

/**
 * Transposes the matrix 'a'.
 *
 * Assuming an NxN matrix 'A':
 * for n = 0 to N - 2
 *	for m = n + 1 to N - 1
 *	 swap A(n,m) with A(m,n)
 */
void transpose_same(mat4 a)
{
	float tmp;
	// n=0, m=[1,3]
	swapf(tmp, a[0*4 + 1], a[1*4 + 0]);
	swapf(tmp, a[0*4 + 2], a[2*4 + 0]);
	swapf(tmp, a[0*4 + 3], a[3*4 + 0]);
	// n=1, m=[2,3]
	swapf(tmp, a[1*4 + 2], a[2*4 + 1]);
	swapf(tmp, a[1*4 + 3], a[3*4 + 1]);
	// n=2, m=[3,3]
	swapf(tmp, a[2*4 + 3], a[3*4 + 2]);
}

/**
 * Store the cross product of 'a x b' in 'v'.
 */
void cross(vec4 v, vec4 a, vec4 b)
{
	v[0] = a[1]*b[2] - a[2]*b[1];
	v[1] = a[2]*b[0] - a[0]*b[2];
	v[2] = a[0]*b[1] - a[1]*b[0];
}

void adjugate(mat4 m, const mat4 a)
{
	m[ 0] = -a[ 7]*a[10]*a[13]+a[ 6]*a[11]*a[13]+a[ 7]*a[ 9]*a[14]-a[ 5]*a[11]*a[14]-a[ 6]*a[ 9]*a[15]+a[ 5]*a[10]*a[15];
	m[ 1] =  a[ 3]*a[10]*a[13]-a[ 2]*a[11]*a[13]-a[ 3]*a[ 9]*a[14]+a[ 1]*a[11]*a[14]+a[ 2]*a[ 9]*a[15]-a[ 1]*a[10]*a[15];
	m[ 2] = -a[ 3]*a[ 6]*a[13]+a[ 2]*a[ 7]*a[13]+a[ 3]*a[ 5]*a[14]-a[ 1]*a[ 7]*a[14]-a[ 2]*a[ 5]*a[15]+a[ 1]*a[ 6]*a[15];
	m[ 3] =  a[ 3]*a[ 6]*a[ 9]-a[ 2]*a[ 7]*a[ 9]-a[ 3]*a[ 5]*a[10]+a[ 1]*a[ 7]*a[10]+a[ 2]*a[ 5]*a[11]-a[ 1]*a[ 6]*a[11];
	m[ 4] =  a[ 7]*a[10]*a[12]-a[ 6]*a[11]*a[12]-a[ 7]*a[ 8]*a[14]+a[ 4]*a[11]*a[14]+a[ 6]*a[ 8]*a[15]-a[ 4]*a[10]*a[15];
	m[ 5] = -a[ 3]*a[10]*a[12]+a[ 2]*a[11]*a[12]+a[ 3]*a[ 8]*a[14]-a[ 0]*a[11]*a[14]-a[ 2]*a[ 8]*a[15]+a[ 0]*a[10]*a[15];
	m[ 6] =  a[ 3]*a[ 6]*a[12]-a[ 2]*a[ 7]*a[12]-a[ 3]*a[ 4]*a[14]+a[ 0]*a[ 7]*a[14]+a[ 2]*a[ 4]*a[15]-a[ 0]*a[ 6]*a[15];
	m[ 7] = -a[ 3]*a[ 6]*a[ 8]+a[ 2]*a[ 7]*a[ 8]+a[ 3]*a[ 4]*a[10]-a[ 0]*a[ 7]*a[10]-a[ 2]*a[ 4]*a[11]+a[ 0]*a[ 6]*a[11];
	m[ 8] = -a[ 7]*a[ 9]*a[12]+a[ 5]*a[11]*a[12]+a[ 7]*a[ 8]*a[13]-a[ 4]*a[11]*a[13]-a[ 5]*a[ 8]*a[15]+a[ 4]*a[ 9]*a[15];
	m[ 9] =  a[ 3]*a[ 9]*a[12]-a[ 1]*a[11]*a[12]-a[ 3]*a[ 8]*a[13]+a[ 0]*a[11]*a[13]+a[ 1]*a[ 8]*a[15]-a[ 0]*a[ 9]*a[15];
	m[10] = -a[ 3]*a[ 5]*a[12]+a[ 1]*a[ 7]*a[12]+a[ 3]*a[ 4]*a[13]-a[ 0]*a[ 7]*a[13]-a[ 1]*a[ 4]*a[15]+a[ 0]*a[ 5]*a[15];
	m[11] =  a[ 3]*a[ 5]*a[ 8]-a[ 1]*a[ 7]*a[ 8]-a[ 3]*a[ 4]*a[ 9]+a[ 0]*a[ 7]*a[ 9]+a[ 1]*a[ 4]*a[11]-a[ 0]*a[ 5]*a[11];
	m[12] =  a[ 6]*a[ 9]*a[12]-a[ 5]*a[10]*a[12]-a[ 6]*a[ 8]*a[13]+a[ 4]*a[10]*a[13]+a[ 5]*a[ 8]*a[14]-a[ 4]*a[ 9]*a[14];
	m[13] = -a[ 2]*a[ 9]*a[12]+a[ 1]*a[10]*a[12]+a[ 2]*a[ 8]*a[13]-a[ 0]*a[10]*a[13]-a[ 1]*a[ 8]*a[14]+a[ 0]*a[ 9]*a[14];
	m[14] =  a[ 2]*a[ 5]*a[12]-a[ 1]*a[ 6]*a[12]-a[ 2]*a[ 4]*a[13]+a[ 0]*a[ 6]*a[13]+a[ 1]*a[ 4]*a[14]-a[ 0]*a[ 5]*a[14];
	m[15] = -a[ 2]*a[ 5]*a[ 8]+a[ 1]*a[ 6]*a[ 8]+a[ 2]*a[ 4]*a[ 9]-a[ 0]*a[ 6]*a[ 9]-a[ 1]*a[ 4]*a[10]+a[ 0]*a[ 5]*a[10];
}

float determinant(const mat4 m)
{
	return +m[ 3]*m[ 6]*m[ 9]*m[12]
		   -m[ 2]*m[ 7]*m[ 9]*m[12]
		   -m[ 3]*m[ 5]*m[10]*m[12]
		   +m[ 1]*m[ 7]*m[10]*m[12]
		   +m[ 2]*m[ 5]*m[11]*m[12]
		   -m[ 1]*m[ 6]*m[11]*m[12]
		   -m[ 3]*m[ 6]*m[ 8]*m[13]
		   +m[ 2]*m[ 7]*m[ 8]*m[13]
		   +m[ 3]*m[ 4]*m[10]*m[13]
		   -m[ 0]*m[ 7]*m[10]*m[13]
		   -m[ 2]*m[ 4]*m[11]*m[13]
		   +m[ 0]*m[ 6]*m[11]*m[13]
		   +m[ 3]*m[ 5]*m[ 8]*m[14]
		   -m[ 1]*m[ 7]*m[ 8]*m[14]
		   -m[ 3]*m[ 4]*m[ 9]*m[14]
		   +m[ 0]*m[ 7]*m[ 9]*m[14]
		   +m[ 1]*m[ 4]*m[11]*m[14]
		   -m[ 0]*m[ 5]*m[11]*m[14]
		   -m[ 2]*m[ 5]*m[ 8]*m[15]
		   +m[ 1]*m[ 6]*m[ 8]*m[15]
		   +m[ 2]*m[ 4]*m[ 9]*m[15]
		   -m[ 0]*m[ 6]*m[ 9]*m[15]
		   -m[ 1]*m[ 4]*m[10]*m[15]
		   +m[ 0]*m[ 5]*m[10]*m[15];
}

void mult_scalar(mat4 m, const mat4 a, const float s)
{
	m[ 0] = a[ 0]*s; m[ 1] = a[ 1]*s; m[ 2] = a[ 2]*s; m[ 3] = a[ 3]*s;
	m[ 4] = a[ 4]*s; m[ 5] = a[ 5]*s; m[ 6] = a[ 6]*s; m[ 7] = a[ 7]*s;
	m[ 8] = a[ 8]*s; m[ 9] = a[ 9]*s; m[10] = a[10]*s; m[11] = a[11]*s;
	m[12] = a[12]*s; m[13] = a[13]*s; m[14] = a[14]*s; m[15] = a[15]*s;
}

void mult_scalar_same(mat4 m, const float s)
{
	mult_scalar(m, m, s);
}

void add(mat4 a, const mat4 b)
{
	a[ 0] += b[ 0]; a[ 1] += b[ 1]; a[ 2] += b[ 2]; a[ 3] += b[ 3];
	a[ 4] += b[ 4]; a[ 5] += b[ 5]; a[ 6] += b[ 6]; a[ 7] += b[ 7];
	a[ 8] += b[ 8]; a[ 9] += b[ 9]; a[10] += b[10]; a[11] += b[11];
	a[12] += b[12]; a[13] += b[13]; a[14] += b[14]; a[15] += b[15];
}

/**
 * If one exists, stores the inverse of matrix 'a' in 'm'.
 *
 * @param m Output matrix.
 * @param a Input matrix.
 * @return	-1 if no inverse exists, 0 otherwise.
 */
int inverse(mat4 m, const mat4 a)
{
	float det = determinant(a);
	if(det == 0) {
		return -1;
	}
	// Store adjugate of a in m
	adjugate(m, a);
	// Turn m into inverse of a now
	mult_scalar_same(m, 1/det);
	return 0;
}

/**
 * Copy the contents of matrix 'a' into 'm'.
 */
void copym(mat4 m, const mat4 a)
{
	memcpy(m, a, sizeof(float) * 16);
}

/**
 * Copy the contents of vector 'a' into 'm'.
 */
void copyv(vec4 v, const vec4 a)
{
	memcpy(v, a, sizeof(float) * 4);
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

void set2f(vec2 v, const float x, const float y)
{
	v[0] = x; v[1] = y;
}

void set3f(vec3 v, const float x, const float y, const float z)
{
	v[0] = x; v[1] = y; v[2] = z;
}

void set4f(vec4 v, const float x, const float y, const float z, const float w)
{
	v[0] = x; v[1] = y; v[2] = z; v[3] = w;
}
