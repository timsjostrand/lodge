#ifndef _INPUT_H
#define _INPUT_H

#include <GLFW/glfw3.h>

struct input;

typedef void (*input_callback_t)(struct input *input, GLFWwindow *window,
		int key, int scancode, int action, int mods);

typedef void (*input_char_callback_t)(struct input *input, GLFWwindow *window,
		unsigned int key, int mods);

struct input {
	int							enabled;
	input_callback_t			callback;
	input_char_callback_t		char_callback;
	int							keys[GLFW_KEY_LAST];		/* Key status of current frame. */
	int							last_keys[GLFW_KEY_LAST];	/* Key status of last frame. */
};

int		input_init(struct input *input, GLFWwindow *window,
				input_callback_t key_callback, input_char_callback_t char_callback);
void	input_think(struct input *input, float delta_time);

void	input_window_to_view(float win_x, float win_y, float win_w, float win_h,
				float view_w, float view_h, float *x, float *y);
void	input_window_get_cursor(GLFWwindow *window, float *x, float *y);
void	input_view_get_cursor(GLFWwindow *window, const float view_w,
				const float view_h, float *x, float *y);

int		key_down(int key);
int		key_pressed(int key);
int		key_released(int key);

#endif
