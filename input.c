/**
 * Keyboard input.
 *
 * Authors: Tim Sjöstrand <tim.sjostrand@gmail.com>
 *			Johan Yngman <johan.yngman@gmail.com>
 */

#include <stdio.h>
#include <string.h>
#include <GLFW/glfw3.h>

#include "log.h"
#include "graphics.h"
#include "input.h"
#include "core.h"

void input_glfw_key_func(GLFWwindow *window, int key, int scancode, int action, int mods);
void input_glfw_char_func(GLFWwindow *window, unsigned int key, int mods);
void input_glfw_mousebutton_callback(GLFWwindow *window, int button, int action, int mods);

int input_init(struct input *input, GLFWwindow *window,
	input_callback_t key_callback, input_char_callback_t char_callback, mousebutton_callback_t mousebutton_callback)
{
	input->enabled = 1;
	input->callback = key_callback;
	input->char_callback = char_callback;
	input->mousebutton_callback = mousebutton_callback;
	input_global = input;
	glfwSetKeyCallback(window, &input_glfw_key_func);
	glfwSetCharModsCallback(window, &input_glfw_char_func);
	glfwSetMouseButtonCallback(window, &input_glfw_mousebutton_callback);
	return GRAPHICS_OK;
}

int key_down(int key)
{
	return input_global && input_global->keys[key];
}

int key_pressed(int key)
{
	return input_global && (input_global->keys[key] && !input_global->last_keys[key]);
}

int key_released(int key)
{
	return input_global && (!input_global->keys[key] && input_global->last_keys[key]);
}

void input_glfw_mousebutton_callback(GLFWwindow *window, int button, int action, int mods)
{
	input_global->mousebutton_callback(window, button, action, mods);
}

void input_glfw_key_func(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	/* Sanity check */
	if(key < 0 || key >= GLFW_KEY_LAST) {
		debugf("Input", "Invalid key: %d (scancode=%d)\n", key, scancode);
		return;
	}

	if(!input_global) {
		errorf("Input", "ERROR: input_init() not called\n");
		return;
	}

	if(input_global->enabled) {
		/* Only care about 'up'/'down', regard 'repeat' as 'down'. */
		input_global->keys[key] = !(action == GLFW_RELEASE);
	}

	if(input_global->callback) {
		input_global->callback(input_global, window, key, scancode, action, mods);
	}
}

void input_glfw_char_func(GLFWwindow *window, unsigned int key, int mods)
{
	if(!input_global) {
		printf("ERROR: input_init() not called\n");
		return;
	}
	if(input_global->char_callback) {
		input_global->char_callback(input_global, window, key, mods);
	}
}

void input_think(struct input *input, float delta_time)
{
	/* Remember what keys were pressed the last frame. */
	for(int i = 0; i < GLFW_KEY_LAST; i++) {
		input->last_keys[i] = input->keys[i];
	}
}

/**
 * Converts window corrdinates to view coordinates.
 */
void input_window_to_view(float win_x, float win_y, float win_w, float win_h, float *x, float *y)
{
	float vx, vy, vw, vh;
	core_get_viewport(&vx, &vy, &vw, &vh);

	float dw = vw - win_w;
	float dh = vh - win_h;

	win_x = win_x + vx + dw;
	win_y = win_y + vy + dh;
	win_w = vw;
	win_h = vh;

	mat4 projection;
	inverse(projection, core_global->graphics.projection);

	vec4 in_pos;
	in_pos[0] = 2.0f * win_x / win_w;
	in_pos[1] = 2.0f * (1.0f - win_y / win_h);
	in_pos[2] = 0.0f;
	in_pos[3] = 1.0f;

	vec4 out_pos;
	mult_vec4(out_pos, projection, in_pos);
	*x = out_pos[0];
	*y = out_pos[1];
}

void input_window_get_cursor(GLFWwindow *window, float *x, float *y)
{
	
	double tmp_x = 0, tmp_y = 0;
	glfwGetCursorPos(window, &tmp_x, &tmp_y);
	*x = (float) tmp_x;
	*y = (float) tmp_y;
}

void input_view_get_cursor(GLFWwindow *window, float *x, float *y)
{
	float win_x = 0, win_y = 0;
	input_window_get_cursor(window, &win_x, &win_y);
	int win_w = 0, win_h = 0;
	glfwGetWindowSize(window, &win_w, &win_h);
	/* Convert screen space => game space. */
	input_window_to_view(win_x, win_y, win_w, win_h, x, y);
}
