#ifndef _LODGE_WINDOW_H
#define _LODGE_WINDOW_H

#define LODGE_WINDOW_MODE_FULLSCREEN	0
#define LODGE_WINDOW_MODE_BORDERLESS	1
#define LODGE_WINDOW_MODE_WINDOWED		2

#define LODGE_CURSOR_MODE_NORMAL		0x00034001
#define LODGE_CURSOR_MODE_HIDDEN		0x00034002
#define LODGE_CURSOR_MODE_DISABLED		0x00034003

typedef int lodge_window_t;

typedef void(*lodge_window_create_callback_t)(lodge_window_t window);
typedef void(*lodge_window_mousebutton_callback_t)(lodge_window_t window, int button, int action, int mods);
typedef void(*lodge_window_input_callback_t)(lodge_window_t window, int key, int scancode, int action, int mods);
typedef void(*lodge_window_input_char_callback_t)(lodge_window_t window, unsigned int key, int mods);
typedef void(*lodge_window_resize_callback_t)(lodge_window_t window, int width, int height);

void			lodge_window_initialize();
void			lodge_window_shutdown();

lodge_window_t	lodge_window_create(const char *title, int window_width, int window_height, int window_mode, lodge_window_create_callback_t create_callback);
void			lodge_window_destroy(lodge_window_t window);
void			lodge_window_update(lodge_window_t window);

void			lodge_window_set_mode(lodge_window_t window, int window_mode);
int				lodge_window_get_mode(lodge_window_t window);
void			lodge_window_toggle_fullscreen(lodge_window_t window);

void			lodge_window_set_cursor_mode(lodge_window_t window, int value);

void			lodge_window_set_mousebutton_callback(lodge_window_t window, lodge_window_mousebutton_callback_t callback);
void			lodge_window_set_input_callback(lodge_window_t window, lodge_window_input_callback_t callback);
void			lodge_window_set_input_char_callback(lodge_window_t window, lodge_window_input_char_callback_t callback);
void			lodge_window_set_resize_callback(lodge_window_t window, lodge_window_resize_callback_t callback);
void			lodge_window_set_create_callback(lodge_window_t window, lodge_window_resize_callback_t callback);

void			lodge_window_set_userdata(lodge_window_t window, void* userdata);
void*			lodge_window_get_userdata(lodge_window_t window);

void			lodge_window_get_size(lodge_window_t window, int* width, int* height);
void			lodge_window_get_cursor(lodge_window_t window, float* x, float* y);

double			lodge_window_get_time();
void			lodge_window_get_screen_size(int* width, int* height);

int				lodge_window_is_open(lodge_window_t window);
int				lodge_window_is_focused(lodge_window_t window);

#endif
