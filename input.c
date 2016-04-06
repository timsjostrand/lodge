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

