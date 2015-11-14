#ifndef CORE_H_
#define CORE_H_

#include "log.h"
#include "math4.h"
#include "vfs.h"
#include "graphics.h"
#include "input.h"
#include "sound.h"
#include "monotext.h"
#include "console.h"

typedef void (*core_load_t)(void);
typedef void (*core_init_t)(void);
typedef void (*core_init_memory_t)(void*);
typedef void (*core_release_t)(void);
typedef void (*core_console_init_t)(struct console *);

#define core_debug(...) debugf("Core", __VA_ARGS__)
#define core_error(...) errorf("Core", __VA_ARGS__)

struct core_textures {
	GLuint	none;
};

struct core {
	float					view_width;
	float					view_height;
	struct graphics			graphics;
	struct core_textures	textures;
	struct input			input;
	struct sound			sound;
	vec3					*sound_listener;
	float					sound_distance_max;
	/* Callbacks. */
	core_load_t				load_callback;
	core_init_t				init_callback;
	core_init_memory_t		init_memory_callback;
	core_release_t			release_callback;
	think_func_t			think_callback;
	render_func_t			render_callback;
	input_callback_t		key_callback;
	input_char_callback_t	char_callback;
	fps_func_t				fps_callback;
	/* Console. */
	core_console_init_t		console_init_callback;
	struct shader			*console_shader;
	struct console			console;
	/* Resources. */
	struct monofont			font_console;

	void*					game_memory;
};

struct core core;

/* Pre-init. */
void core_set_init_memory_callback(core_init_memory_t init_memory_callback);
void core_set_think_callback(think_func_t think_callback);
void core_set_render_callback(render_func_t render_callback);
void core_set_key_callback(input_callback_t key_callback);
void core_set_char_callback(input_char_callback_t char_callback);
void core_set_asset_callbacks(core_load_t load_callback,
		core_init_t init_callback, core_release_t release_callback);
void core_set_fps_callback(fps_func_t fps_callback);
void core_set_console_init_callback(core_console_init_t console_init_callback);
void core_set_up_sound(vec3 *sound_listener, float distance_max);
void core_set_up_console(core_console_init_t console_init_callback,
		struct shader *console_shader);

void core_setup(const char *title, int view_width, int view_height,
	int window_width, int window_height, int windowed, size_t game_memory_size);

void core_run();
void core_reload();

#endif
