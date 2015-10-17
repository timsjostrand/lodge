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
typedef void (*core_release_t)(void);

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
	struct shader			*shader_console;
	struct console			console;
	struct sound			sound;
	vec3					*listener;
	/* Callbacks. */
	core_load_t				load_callback;
	core_init_t				init_callback;
	core_release_t			release_callback;
	think_func_t			think_callback;
	render_func_t			render_callback;
	input_callback_t		key_callback;
	input_char_callback_t	char_callback;
	/* Resources. */
	struct monofont			font_console;
};

struct core core;

void core_init(int view_width, int view_height, int windowed,
		const char *mount_path,
		vec3 *listener, float sound_distance_max,
		struct shader *shader_console,
		think_func_t			think_callback,
		render_func_t			render_callback,
		fps_func_t				fps_callback,
		core_load_t				load_callback,
		core_init_t				init_callback,
		core_release_t			release_callback,
		input_callback_t		key_callback,
		input_char_callback_t	char_callback
);

#endif
