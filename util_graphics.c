/**
 * Important stuff
 */

#include <GLFW/glfw3.h>

#include "util_graphics.h"
#include "math4.h"

void util_get_viewport(float* x, float* y, float* w, float* h)
{
	float buffer[4];
	glGetFloatv(GL_VIEWPORT, buffer);

	*x = buffer[0];
	*y = buffer[1];
	*w = buffer[2];
	*h = buffer[3];
}

/**
 * Converts window corrdinates to view coordinates.
 */
void util_window_to_view(float win_x, float win_y, float win_w, float win_h, float *x, float *y, mat4 projection)
{
	float vx, vy, vw, vh;
	util_get_viewport(&vx, &vy, &vw, &vh);

	float dw = vw - win_w;
	float dh = vh - win_h;

	win_x = win_x + vx + dw;
	win_y = win_y + vy + dh;
	win_w = vw;
	win_h = vh;

	mat4 projection_inv;
	inverse(projection_inv, projection);

	vec4 in_pos;
	in_pos[0] = 2.0f * win_x / win_w;
	in_pos[1] = 2.0f * (1.0f - win_y / win_h);
	in_pos[2] = 0.0f;
	in_pos[3] = 1.0f;

	vec4 out_pos;
	mult_vec4(out_pos, projection_inv, in_pos);
	*x = out_pos[0];
	*y = out_pos[1];
}

void util_window_get_cursor(GLFWwindow *window, float *x, float *y)
{
	double tmp_x = 0, tmp_y = 0;
	glfwGetCursorPos(window, &tmp_x, &tmp_y);
	*x = (float) tmp_x;
	*y = (float) tmp_y;
}

void util_view_get_cursor(GLFWwindow *window, float *x, float *y, mat4 projection)
{
	float win_x = 0, win_y = 0;
	util_window_get_cursor(window, &win_x, &win_y);
	int win_w = 0, win_h = 0;
	glfwGetWindowSize(window, &win_w, &win_h);
	/* Convert screen space => game space. */
	util_window_to_view(win_x, win_y, win_w, win_h, x, y, projection);
}
