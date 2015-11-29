#ifndef CORE_H_
#define CORE_H_

#include "game.h"
#include "log.h"
#include "math4.h"
#include "vfs.h"
#include "input.h"
#include "sound.h"
#include "monotext.h"
#include "console.h"

#include "graphics.h"

struct core;

typedef void (*core_load_t)(void);
typedef void (*core_init_t)(void);
typedef void (*core_init_memory_t)(struct shared_memory* shared, int reload);
typedef void(*core_release_t)(void);
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
	mousebutton_callback_t	mousebutton_callback;
	fps_func_t				fps_callback;
	/* Console. */
	core_console_init_t		console_init_callback;
	struct shader			*console_shader;
	struct console			console;
	/* Resources. */
	struct monofont			font_console;

	struct shared_memory	shared_memory;
};

struct core *core_global;

/* Pre-init. */
void core_set_init_memory_callback(struct core* core, core_init_memory_t init_memory_callback);
void core_set_think_callback(struct core* core, think_func_t think_callback);
void core_set_render_callback(struct core* core, render_func_t render_callback);
void core_set_key_callback(struct core* core, input_callback_t key_callback);
void core_set_mousebutton_callback(struct core* core, mousebutton_callback_t key_callback);
void core_set_char_callback(struct core* core, input_char_callback_t char_callback);
void core_set_asset_callbacks(struct core* core, core_load_t load_callback,
		core_init_t init_callback, core_release_t release_callback);
void core_set_fps_callback(struct core* core, fps_func_t fps_callback);
void core_set_console_init_callback(struct core* core, core_console_init_t console_init_callback);
void core_set_up_sound(struct core* core, vec3 *sound_listener, float distance_max);

void core_setup(struct core* core, const char *title, int view_width, int view_height,
	int window_width, int window_height, int windowed, size_t game_memory_size);

void core_run(struct core* core);
void core_reload(struct core* core);

#endif
