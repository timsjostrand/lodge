#include "lodge_noise.h"

#include "simplexnoise1234.h"

float lodge_noise_simplex_1d(float x)
{
	return snoise1(x);
}

float lodge_noise_simplex_2d(float x, float y)
{
	return snoise2(x, y);
}

float lodge_noise_simplex_3d(float x, float y, float z)
{
	return snoise3(x, y, z);
}

float lodge_noise_simplex_4d(float x, float y, float z, float w)
{
	return snoise4(x, y, z, w);
}