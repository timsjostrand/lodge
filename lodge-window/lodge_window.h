#ifndef _LODGE_WINDOW_H
#define _LODGE_WINDOW_H

#include "lodge_plugin.h"

#define LODGE_WINDOW_MODE_FULLSCREEN	0
#define LODGE_WINDOW_MODE_BORDERLESS	1
#define LODGE_WINDOW_MODE_WINDOWED		2

#define LODGE_CURSOR_MODE_NORMAL		0x00034001
#define LODGE_CURSOR_MODE_HIDDEN		0x00034002
#define LODGE_CURSOR_MODE_DISABLED		0x00034003

#define LODGE_WINDOWS_MAX				128

struct lodge_window;
typedef struct lodge_window* lodge_window_t;

typedef void				(*lodge_window_mousebutton_callback_t)(lodge_window_t window, int button, int action, int mods, void *userdata);
typedef void				(*lodge_window_scroll_callback_t)(lodge_window_t window, double x, double y, void *userdata);
typedef void				(*lodge_window_input_callback_t)(lodge_window_t window, int key, int scancode, int action, int mods, void *userdata);
typedef void				(*lodge_window_input_char_callback_t)(lodge_window_t window, unsigned int key, int mods, void *userdata);
typedef void				(*lodge_window_resize_callback_t)(lodge_window_t window, int width, int height, void *userdata);

struct lodge_windows
{
	strview_t				library;
	lodge_window_t			windows[LODGE_WINDOWS_MAX];
	int						windows_count;
};

struct lodge_gfx;

struct lodge_plugin_desc	lodge_plugin_windows();

lodge_window_t				lodge_window_new(struct lodge_windows *windows, const char *title, int window_width, int window_height, int window_mode);
void						lodge_window_free(lodge_window_t window);
void						lodge_window_update(lodge_window_t window);
void						lodge_window_render(lodge_window_t window);

void						lodge_window_set_mode(lodge_window_t window, int window_mode);
int							lodge_window_get_mode(lodge_window_t window);
void						lodge_window_toggle_fullscreen(lodge_window_t window);

struct lodge_ret			lodge_window_set_renderer(lodge_window_t window, struct lodge_gfx *gfx);
void						lodge_window_set_cursor_mode(lodge_window_t window, int value);
void						lodge_window_set_should_close(lodge_window_t window, int value);

void						lodge_window_set_mousebutton_callback(lodge_window_t window, lodge_window_mousebutton_callback_t callback, void *userdata);
void						lodge_window_set_scroll_callback(lodge_window_t window, lodge_window_scroll_callback_t callback, void *userdata);
void						lodge_window_set_input_callback(lodge_window_t window, lodge_window_input_callback_t callback, void *userdata);
void						lodge_window_set_input_char_callback(lodge_window_t window, lodge_window_input_char_callback_t callback, void *userdata);
void						lodge_window_set_resize_callback(lodge_window_t window, lodge_window_resize_callback_t callback, void *userdata);

void						lodge_window_get_size(lodge_window_t window, int *width, int *height);
void						lodge_window_get_cursor(lodge_window_t window, float *x, float *y);
int							lodge_window_get_mouse_button(lodge_window_t window, int mouse_button);

void						lodge_window_get_screen_size(int* width, int* height);
void						lodge_window_set_vsync_enabled(lodge_window_t window, int vsync);

int							lodge_window_is_open(lodge_window_t window);
int							lodge_window_is_focused(lodge_window_t window);

struct lodge_input*			lodge_window_get_input(lodge_window_t);

// FIXME(TS): use lodge_input instead
int							lodge_window_key_down(lodge_window_t window, int key);

strview_t					lodge_window_get_clipboard(lodge_window_t window);
void						lodge_window_set_clipboard(lodge_window_t window, strview_t str);

#endif
