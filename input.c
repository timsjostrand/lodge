/**
 * Keyboard input.
 *
 * Authors: Tim Sjöstrand <tim.sjostrand@gmail.com>
 *			Johan Yngman <johan.yngman@gmail.com>
 */

#include <stdio.h>
#include <string.h>
#include <GLFW/glfw3.h>

#include "graphics.h"
#include "input.h"

struct input *input_global = NULL;

void input_glfw_key_func(GLFWwindow *window, int key, int scancode, int action, int mods);
void input_glfw_char_func(GLFWwindow *window, unsigned int key, int mods);

int input_init(struct input *input, GLFWwindow *window,
		input_callback_t key_callback, input_char_callback_t char_callback)
{
	input->enabled = 1;
	input->callback = key_callback;
	input->char_callback = char_callback;
	input_global = input;
	glfwSetKeyCallback(window, &input_glfw_key_func);
	glfwSetCharModsCallback(window, &input_glfw_char_func);
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

void input_glfw_key_func(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	/* Sanity check */
	if(key < 0 || key >= GLFW_KEY_LAST) {
		printf("Invalid key: %d (scancode=%d)\n", key, scancode);
		return;
	}

	if(!input_global) {
		printf("ERROR: input_init() not called\n");
		return;
	}

	if(input_global->enabled) {
		/* Only care about 'up'/'down', regard 'repeat' as 'down'. */
		input_global->keys[key] = !(action == 0);
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
	if(key > 255) {
		printf("ERROR: Non-convertible Unicode char: %lc\n", key);
		return;
	}
	if(input_global->char_callback) {
		input_global->char_callback(input_global, window, (unsigned char) key, mods);
	}
}

void input_think(struct input *input, float delta_time)
{
	/* Remember what keys were pressed the last frame. */
	memcpy(input->last_keys, input->keys, GLFW_KEY_LAST);
}
