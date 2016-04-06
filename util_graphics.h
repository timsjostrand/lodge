#ifndef _UTIL_GRAPHICS_H
#define _UTIL_GRAPHICS_H

#include "math4.h"

void	util_get_viewport(float* x, float* y, float* w, float* h);
void	util_window_to_view(float win_x, float win_y, float win_w, float win_h, float *x, float *y,
			mat4 projection);
void	util_window_get_cursor(GLFWwindow *window, float *x, float *y);
void	util_view_get_cursor(GLFWwindow *window, float *x, float *y, mat4 projection);

#endif
