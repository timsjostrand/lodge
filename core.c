/**
 * Set up common game functionality.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>

#include "time.h"
#include "math4.h"
#include "graphics.h"
#include "input.h"
#include "sound.h"
#include "monotext.h"
#include "texture.h"

#include "core.h"
#include "core_argv.h"
#include "core_console.h"

struct core core = { 0 };

static void core_load()
{
	/* Create a square white texture for texturing empty sprites with. */
	texture_white(&core.textures.none);

	/* Load fonts used for the console. */
	monofont_new(&core.font_console, "04B03_8px.png", 8, 8, -2, 0);

	/* Game specific load. */
	if(core.load_callback != NULL) {
		core.load_callback();
	}
}

static void core_release()
{
	texture_free(core.textures.none);
	monofont_free(&core.font_console);
	console_free(&core.console);

	/* Release game specific resources. */
	if(core.release_callback != NULL) {
		core.release_callback();
	}
}

static void core_assets_init()
{
	/* Load console. */
	console_new(&core.console, &core.font_console, core.view_width, 16, &core.textures.none);
	core_console_init(&core.graphics, &core.console);
	if(core.console_init_callback != NULL) {
		core.console_init_callback(&core.console);
	}
	console_parse_conf(&core.console, &core.console.conf);

	/* Game specific init. */
	if(core.init_callback != NULL) {
		core.init_callback();
	}
}

static void core_think(struct graphics *g, float delta_time)
{
	/* Common think functions. */
	vfs_filewatch();
	console_think(&core.console, delta_time);
	sound_think(&core.sound, delta_time);

	/* Game specific think. */
	if(core.think_callback != NULL) {
		core.think_callback(g, delta_time);
	}
	
	/* Input must think after game logic. */
	input_think(&core.input, delta_time);
}

static void core_render(struct graphics *g, float delta_time)
{
	/* Game specific render. */
	if(core.render_callback != NULL) {
		core.render_callback(g, delta_time);
	}

	/* Render the console. */
	if(core.console.focused) {
		console_render(&core.console, core.console_shader, g);
	}
}

static void core_key_callback(struct input *input, GLFWwindow *window, int key, int scancode, int action, int mods)
{
	/* Input control characters into console. */
	if(core.console.focused && key >= 256) {
		console_input_feed_control(&core.console, key, scancode, action, mods);
	}

	if(!core.console.focused) {
		if(core.key_callback != NULL) {
			core.key_callback(input, window, key, scancode, action, mods);
		}
	}
}

static void core_char_callback(struct input *input, GLFWwindow *window,
		unsigned int key, int mods)
{
	/* Input localized text into console. */
	if(core.console.focused) {
		console_input_feed_char(&core.console, key, mods);
	}

	/* Toggle console. */
	if(key == CONSOLE_CHAR_FOCUS) {
		console_toggle_focus(&core.console);
		input->enabled = !core.console.focused;
	}

	if(core.char_callback != NULL) {
		core.char_callback(input, window, key, mods);
	}
}

void core_set_fps_callback(fps_func_t fps_callback)
{
	core.fps_callback = fps_callback;
}

void core_set_asset_callbacks(core_load_t load_callback,
		core_init_t init_callback, core_release_t release_callback)
{
	core.load_callback = load_callback;
	core.init_callback = init_callback;
	core.release_callback = release_callback;
}

void core_set_key_callback(input_callback_t key_callback)
{
	core.key_callback = key_callback;
}

void core_set_char_callback(input_char_callback_t char_callback)
{
	core.char_callback = char_callback;
}

void core_set_up_sound(vec3 *listener, float distance_max)
{
	core.sound_listener = listener;
	core.sound_distance_max = distance_max;
}

void core_set_up_console(core_console_init_t console_init_callback, struct shader *console_shader)
{
	core.console_init_callback = console_init_callback;
	core.console_shader = console_shader;
}

/**
 * @param load_callback		Responsible for registering all the VFS callbacks
 *							required for the game. After load_callback has been
 *							run, assets are immediately loaded.
 * @param init_callback		Runs after assets are loaded, and should set up game
 *							objects to their initial state.
 * @param release_callback	Runs just before the game quits, and should release
 *							all assets and free dynamically allocated memory.
 */
void core_run(const char *title, int view_width, int view_height,
		int window_width, int window_height, int windowed,
		const char *mount_path, think_func_t think_callback,
		render_func_t render_callback)
{
	/* Store global references. */
	core.view_width = view_width;
	core.view_height = view_height;
	core.render_callback = render_callback;
	core.think_callback = think_callback;

	/* Seed random number generator. */
	srand(time(NULL));

	/* Start the virtual file system */
	vfs_init(mount_path);

	/* Set up sound */
	sound_init(&core.sound, (float *) core.sound_listener, core.sound_distance_max);

	/* Set up graphics. */
	int ret = graphics_init(&core.graphics, &core_think, &core_render,
			core.fps_callback, view_width, view_height, windowed, title,
			window_width, window_height);

	if(ret != GRAPHICS_OK) {
		core_error("Graphics initialization failed (%d)\n", ret);
		graphics_free(&core.graphics);
		exit(ret);
	}

	/* Get input events. */
	ret = input_init(&core.input, core.graphics.window, &core_key_callback,
			&core_char_callback);

	if(ret != GRAPHICS_OK) {
		core_error("Input initialization failed (%d)\n", ret);
		graphics_free(&core.graphics);
		exit(ret);
	}

	/* Load all assets */
	core_load();
	vfs_run_callbacks();

	/* Setup. */
	core_assets_init();

	/* Loop until the user closes the window */
	graphics_loop(&core.graphics);

	/* Release assets. */
	core_release();

	/* Free OpenAL. */
	sound_free(&core.sound);

	/* If we reach here, quit the core. */
	graphics_free(&core.graphics);

	vfs_shutdown();
}
