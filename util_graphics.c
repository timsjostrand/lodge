/**
 * Graphics helpers.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <GLFW/glfw3.h>

#include "util_graphics.h"
#include "math4.h"

/**
 * Retrieve the OpenGL viewport.
 *
 * @param x	Viewport X destination (or NULL to skip).
 * @param y Viewport Y destination (or NULL to skip).
 * @param w Viewport width destination (or NULL to skip).
 * @param h Viewport height destination (or NULL to skip).
 */
void util_get_viewport(float* x, float* y, float* w, float* h)
{
	float buffer[4];
	glGetFloatv(GL_VIEWPORT, buffer);

	if(x != NULL) {
		*x = buffer[0];
	}

	if(y != NULL) {
		*y = buffer[1];
	}

	if(w != NULL) {
		*w = buffer[2];
	}

	if(h != NULL) {
		*h = buffer[3];
	}
}

/**
 * Converts window corrdinates to view coordinates.
 *
 * @param win_x			X coordinate in window space.
 * @param win_y			Y coordinate in window space.
 * @param win_w			Window width.
 * @param win_h			Window height.
 * @param x				Destination X in view space.
 * @param y				Destination Y in view space.
 * @param projection	View projection matrix.
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

/**
 * Retrieve the cursor position in window space.
 *
 * @param window	GLFW window handle.
 * @param x			X coordinate destination.
 * @param y			Y coordinate destination.
 */
void util_window_get_cursor(GLFWwindow *window, float *x, float *y)
{
	double tmp_x = 0, tmp_y = 0;
	glfwGetCursorPos(window, &tmp_x, &tmp_y);
	*x = (float) tmp_x;
	*y = (float) tmp_y;
}

/**
 * Retrieve cursor position in view space.
 *
 * @param window		GLFW window handle.
 * @param x				X coordinate destination.
 * @param y				Y coordinate destination.
 * @param projection	View projection matrix.
 */
void util_view_get_cursor(GLFWwindow *window, float *x, float *y, mat4 projection)
{
	float win_x = 0, win_y = 0;
	util_window_get_cursor(window, &win_x, &win_y);
	int win_w = 0, win_h = 0;
	glfwGetWindowSize(window, &win_w, &win_h);
	/* Convert screen space => game space. */
	util_window_to_view(win_x, win_y, win_w, win_h, x, y, projection);
}