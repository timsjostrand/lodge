#include "input.h"

#include <stdio.h>
#include <string.h>

#include "log.h"

#include "lodge_window.h"

#if 0
struct input {
	lodge_window_t							window;

	lodge_window_input_callback_t			callback;
	lodge_window_mousebutton_callback_t		mousebutton_callback;
	lodge_window_input_char_callback_t		char_callback;

	int										keys[LODGE_KEY_LAST];		/* Key status of current frame. */
	int										last_keys[LODGE_KEY_LAST];	/* Key status of last frame. */
};

static void input_wrap_mousebutton_callback(lodge_window_t window, int button, int action, int mods, struct input *input)
{
	input->mousebutton_callback(window, button, action, mods, NULL);
}

static void input_wrap_key_func(lodge_window_t window, int key, int scancode, int action, int mods, struct input *input)
{
	/* Sanity check */
	if (key < 0 || key >= LODGE_KEY_LAST) {
		debugf("Input", "Invalid key: %d (scancode=%d)\n", key, scancode);
		return;
	}

	/* Only care about 'up'/'down', regard 'repeat' as 'down'. */
	input->keys[key] = !(action == LODGE_RELEASE);

	if(input->callback) {
		input->callback(window, key, scancode, action, mods, NULL);
	}
}

static void input_wrap_char_func(lodge_window_t window, unsigned int key, int mods, struct input *input)
{
	if(input->char_callback) {
		input->char_callback(window, key, mods);
	}
}

struct input* input_new(lodge_window_t window)
{
	struct input *input = (struct input *) calloc(1, sizeof(struct input));

	input->window = window;

	lodge_window_set_mousebutton_callback(window, &input_wrap_mousebutton_callback, input);
	lodge_window_set_input_char_callback(window, &input_wrap_char_func, input);
	lodge_window_set_input_callback(window, &input_wrap_key_func, input);

	return input;
}

void input_free(struct input *input)
{
	lodge_window_set_mousebutton_callback(input->window, NULL, NULL);
	lodge_window_set_input_callback(input->window, NULL, NULL);
	lodge_window_set_input_char_callback(input->window, NULL, NULL);

	free(input);
}

void input_set_key_callback(struct input *input, lodge_window_input_callback_t callback)
{
	input->callback = callback;
}

void input_set_char_callback(struct input *input, lodge_window_input_char_callback_t callback)
{
	input->char_callback = callback;
}

void input_set_mousebutton_callback(struct input *input, lodge_window_mousebutton_callback_t callback)
{
	input->mousebutton_callback = callback;
}


// FIXME(TS): when to call update?
static void input_update(struct input *input, float delta_time)
{
	/* Remember what keys were pressed the last frame. */
	for(int i = 0; i < LODGE_KEY_LAST; i++) {
		input->last_keys[i] = input->keys[i];
	}
}

int input_key_down(struct input *input, int key)
{
	return input->keys[key];
}

int input_key_pressed(struct input *input, int key)
{
	return (input->keys[key] && !input->last_keys[key]);
}

int input_key_released(struct input *input, int key)
{
	return (!input->keys[key] && input->last_keys[key]);
}

#endif