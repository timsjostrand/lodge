#ifndef _INPUT_H
#define _INPUT_H

#include <GLFW/glfw3.h>

struct input;

typedef void (*input_callback_t)(struct input *input, GLFWwindow *window,
        int key, int scancode, int action, int mods);

struct input {
    input_callback_t    callback;
    int                 keys[GLFW_KEY_LAST];        /* Key status of current frame. */
    int                 last_keys[GLFW_KEY_LAST];   /* Key status of last frame. */
};

int     input_init(struct input *input, GLFWwindow *window);
void    input_think(struct input *input, float delta_time);

int     key_down(int key);
int     key_pressed(int key);
int     key_relased(int key);

#endif
