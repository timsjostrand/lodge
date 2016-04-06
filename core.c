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
#include "assets.h"
#include "vfs.h"

#include "core.h"
#include "core_argv.h"
#include "core_console.h"

static void core_load()
{
	/* Create a square white texture for texturing empty sprites with. */
	texture_white(&core_global->textures.none);

	/* Load fonts used for the console. */
	monofont_new(&core_global->font_console, "04B03_8px.png", 8, 8, -2, 0);

	/* Game specific load. */
	if(core_global->load_callback != NULL) {
		core_global->load_callback();
	}
}

static void core_release()
{
	texture_free(core_global->textures.none);
	monofont_free(&core_global->font_console);
	console_free(&core_global->console);

	/* Release game specific resources. */
	if(core_global->release_callback != NULL) {
		core_global->release_callback();
	}
}

static void core_assets_init()
{
	/* Load console. */
	console_new(&core_global->console, &core_global->font_console, core_global->view_width, 16, &core_global->textures.none,
			core_global->console_shader);

	/* Create core commands. */
	core_console_new(&core_global->console);

	/* Create game-specific commands. */
	if (core_global->console_init_callback != NULL) {
		core_global->console_init_callback(&core_global->console);
	}
	console_parse_conf(&core_global->console, &core_global->console.conf);

	/* Game specific init. */
	if (core_global->init_callback != NULL) {
		core_global->init_callback();
	}
}

static void core_think(struct graphics *g, float delta_time)
{
	/* Common think functions. */
	vfs_filewatch();
	console_think(&core_global->console, delta_time);
	sound_think(&core_global->sound, delta_time);

	/* Game specific think. */
	if(core_global->think_callback != NULL) {
		core_global->think_callback(g, delta_time);
	}

	/* Input must think after game logic. */
	input_think(&core_global->input, delta_time);
}

static void core_render(struct graphics *g, float delta_time)
{
	/* Game specific render. */
	if(core_global->render_callback != NULL) {
		core_global->render_callback(g, delta_time);
	}

	/* Render the console. */
	if(core_global->console.focused) {
		console_render(&core_global->console, core_global->console_shader, g);
	}
}

static void core_key_callback(struct input *input, GLFWwindow *window, int key, int scancode, int action, int mods)
{
	/* Input control characters into console. */
	if(core_global->console.focused && key >= 256) {
		console_input_feed_control(&core_global->console, key, scancode, action, mods);
	}

	if(!core_global->console.focused) {
		if(core_global->key_callback != NULL) {
			core_global->key_callback(input, window, key, scancode, action, mods);
		}
	}
}

static void core_mousebutton_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (core_global->mousebutton_callback != NULL) {
		core_global->mousebutton_callback(window, button, action, mods);
	}
}

static void core_char_callback(struct input *input, GLFWwindow *window,
		unsigned int key, int mods)
{
	/* Input localized text into console. */
	if(core_global->console.focused) {
		console_input_feed_char(&core_global->console, key, mods);
	}

	/* Toggle console. */
	if(key == CONSOLE_CHAR_FOCUS) {
		console_toggle_focus(&core_global->console);
		input->enabled = !core_global->console.focused;
	}

	if(core_global->char_callback != NULL) {
		core_global->char_callback(input, window, key, mods);
	}
}

void core_set_fps_callback(fps_func_t fps_callback)
{
	core_global->fps_callback = fps_callback;
	core_global->graphics.frames.callback = fps_callback;
}

void core_set_asset_callbacks(core_load_t load_callback,
		core_init_t init_callback, core_release_t release_callback)
{
	core_global->load_callback = load_callback;
	core_global->init_callback = init_callback;
	core_global->release_callback = release_callback;
}

void core_set_key_callback(input_callback_t key_callback)
{
	core_global->key_callback = key_callback;
}

void core_set_mousebutton_callback(mousebutton_callback_t mousebutton_callback)
{
	core_global->mousebutton_callback = mousebutton_callback;
}

void core_set_char_callback(input_char_callback_t char_callback)
{
	core_global->char_callback = char_callback;
}

void core_set_up_sound(vec3 *listener, float distance_max)
{
	core_global->sound_listener = listener;
	core_global->sound_distance_max = distance_max;
}

void core_set_console_init_callback(core_console_init_t console_init_callback)
{
	core_global->console_init_callback = console_init_callback;
}

void core_set_think_callback(think_func_t think_callback)
{
	core_global->think_callback = think_callback;
}

void core_set_render_callback(render_func_t render_callback)
{
	core_global->render_callback = render_callback;
}

void core_set_init_memory_callback(core_init_memory_t init_memory_callback)
{
	core_global->init_memory_callback = init_memory_callback;
}

void core_glfw_resize_callback(GLFWwindow *window, int width, int height)
{
	float ax = width;
	float ay = height;
	float ratio = core_global->view_height / core_global->view_width;

	float max_width = height / ratio;

	if (width < max_width)
	{
		ax = width;
		ay = ax*ratio;
	}
	else
	{
		ax = ay/ratio;
		ay = height;
	}

	float rest_width = width - ax;
	float rest_height = height - ay;

	glViewport(rest_width / 2.0f, rest_height / 2.0f, ax, ay);
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
void core_setup(const char *title, int view_width, int view_height,
		int window_width, int window_height, int window_mode, size_t game_memory_size)
{
	/* Store global references. */
	core_global->view_width = view_width;
	core_global->view_height = view_height;
	/* FIXME: nice way to set this pointer from game library OR engine specific console shader. */
	core_global->console_shader = &assets->shaders.basic_shader;

	/* Set up sound */
	sound_init(&core_global->sound, (float *)core_global->sound_listener, core_global->sound_distance_max);

	/* Allocate game memory */
	core_global->shared_memory.game_memory = malloc(game_memory_size);
	core_global->shared_memory.core = core_global;
	core_global->shared_memory.sound = &core_global->sound;
	core_global->shared_memory.assets = assets;
	core_global->shared_memory.vfs = vfs_global;
	core_global->shared_memory.input = &core_global->input;
	core_global->init_memory_callback(&core_global->shared_memory, 0);

	/* Seed random number generator. */
	srand(time(NULL));

	/* Set up graphics. */
	int ret = graphics_init(&core_global->graphics, &core_think, &core_render,
			core_global->fps_callback, view_width, view_height, window_mode, title,
			window_width, window_height);

	if(ret != GRAPHICS_OK) {
		core_error("Graphics initialization failed (%d)\n", ret);
		graphics_free(&core_global->graphics);
		exit(ret);
	}

	glfwSetFramebufferSizeCallback(core_global->graphics.window, &core_glfw_resize_callback);

	/* Set up ratio. */
	int w, h;
	glfwGetWindowSize(core_global->graphics.window, &w, &h);
	core_glfw_resize_callback(core_global->graphics.window, w, h);

	/* Get input events. */
	ret = input_init(&core_global->input, core_global->graphics.window, &core_key_callback, &core_char_callback, &core_mousebutton_callback);

	if(ret != GRAPHICS_OK) {
		core_error("Input initialization failed (%d)\n", ret);
		graphics_free(&core_global->graphics);
		exit(ret);
	}

	/* Load all assets */
	core_load();
}

void core_run()
{
	/* Setup. */
	core_assets_init();

	/* Loop until the user closes the window */
	graphics_loop(&core_global->graphics);

	/* Release assets. */
	core_release();

	/* Free OpenAL. */
	sound_free(&core_global->sound);

	/* If we reach here, quit the core. */
	graphics_free(&core_global->graphics);

	/* Release game memory */
	free(core_global->shared_memory.game_memory);
}

void core_reload()
{
	core_global->init_memory_callback(&core_global->shared_memory, 1);
}
