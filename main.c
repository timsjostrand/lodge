#include "game.h"

#include "log.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "vfs.h"
#include "assets.h"

#include "core.h"
#include "core_argv.h"
#include "core_reload.h"

typedef struct game_settings* (*game_get_settings_fn_t)();

/* Core singleton. */
struct core core_mem = { 0 };
struct core *core_global = &core_mem;

/* VFS singleton. */
struct vfs vfs_mem = { 0 };
struct vfs *vfs_global = &vfs_mem;

/* Input singleton. */
struct input *input_global = NULL;

/* Assets singleton. */
struct assets assets_mem = { 0 };
struct assets *assets = &assets_mem;

void* game_library = 0;

core_init_t game_init_fn = 0;
core_load_t game_assets_load_fn = 0;
core_release_t game_assets_release_fn = 0;
think_func_t game_think_fn = 0;
render_func_t game_render_fn = 0;
input_callback_t game_key_callback_fn = 0;
fps_func_t game_fps_callback_fn = 0;
core_console_init_t game_console_init_fn = 0;
core_init_memory_t game_init_memory_fn = 0;
game_get_settings_fn_t game_get_settings_fn = NULL;

void* load_shared_library(const char* filename)
{
	void* library = 0;

#ifdef _WIN32
	library = (void*)LoadLibrary(filename);
#else
	library = dlopen(filename, 2);
#endif

	if (!library)
	{
		errorf("Main", "Could not load library %s\n", filename);
	}

    return library;
}

void* load_function(void* library, const char* function_name)
{
	void* function = 0;

#ifdef _WIN32
	function = (void*)GetProcAddress((HINSTANCE)library, function_name);
#else
	function = dlsym(library, function_name);
#endif

	if (!function)
	{
		errorf("Main", "Could not load function %s\n", function_name);
	}

	return function;
}

/**
 * @return 0 on success, non-zero on failure.
 */
int free_shared_library(void* library)
{
#ifdef _WIN32
	return FreeLibrary((HINSTANCE)library) == 0;
#else
	return dlclose(library);
#endif
}

int first_load = 1;

void load_game(const char* filename, unsigned int size, void* data, void* userdata)
{
	debugf("Main", "Loading game library %s\n", filename);

	if (game_library)
	{
		if (game_assets_release_fn)
		{
			game_assets_release_fn();
		}

		free_shared_library(game_library);
		game_library = 0;
	}

	const char* absolute_path = vfs_get_absolute_path(filename);

	if (!absolute_path)
	{
		return;
	}

	game_library = load_shared_library(absolute_path);
	if (game_library)
	{
		game_init_fn = load_function(game_library, "game_init");
		game_assets_load_fn = load_function(game_library, "game_assets_load");
		game_assets_release_fn = load_function(game_library, "game_assets_release");
		game_think_fn = load_function(game_library, "game_think");
		game_render_fn = load_function(game_library, "game_render");
		game_key_callback_fn = load_function(game_library, "game_key_callback");
		game_fps_callback_fn = load_function(game_library, "game_fps_callback");
		game_console_init_fn = load_function(game_library, "game_console_init");
		game_init_memory_fn = load_function(game_library, "game_init_memory");
		game_get_settings_fn = load_function(game_library, "game_get_settings");

		if (game_init_fn &&
			game_assets_load_fn &&
			game_assets_release_fn &&
			game_think_fn &&
			game_render_fn &&
			game_key_callback_fn &&
			game_fps_callback_fn &&
			game_console_init_fn &&
			game_init_memory_fn)
		{
			core_set_asset_callbacks(core_global, game_assets_load_fn, game_init_fn, game_assets_release_fn);
			core_set_key_callback(core_global, game_key_callback_fn);
			core_set_fps_callback(core_global, game_fps_callback_fn);
			core_set_console_init_callback(core_global, game_console_init_fn);
			core_set_think_callback(core_global, game_think_fn);
			core_set_render_callback(core_global, game_render_fn);
			core_set_init_memory_callback(core_global, game_init_memory_fn);

			if (!first_load)
			{
				core_reload(core_global);
			}
		}
	}
}

int main(int argc, char **argv)
{
	/* Parse command line arguments. */
	struct core_argv args = { 0 };
	core_argv_parse(&args, argc, argv);

	/* Start the virtual file system */
	vfs_init(args.mount);

#ifdef LOAD_SHARED
	/* Load game library */
	size_t filesize;
	void* lib = vfs_get_file(args.game, &filesize);
	load_game(args.game, filesize, lib, 0);
	first_load = 0;

	if (!game_library)
	{
		return 0;
	}

	/* Get game settings. */
	struct game_settings *settings = game_get_settings_fn();
#else
	core_set_think_callback(core_global, &game_think);
	core_set_render_callback(core_global, &game_render);
	core_set_asset_callbacks(core_global, &game_assets_load, &game_init, &game_assets_release);
	core_set_key_callback(core_global, &game_key_callback);
	core_set_fps_callback(core_global, &game_fps_callback);
	core_set_init_memory_callback(core_global, &game_init_memory);
	core_set_up_console(core_global, &game_console_init, &assets->shaders.basic_shader);
	struct game_settings *settings = game_get_settings();
#endif

	/* Sound setup */
	core_set_up_sound(core_global, &settings->sound_listener, settings->sound_distance_max);

	/* Initialize subsystems and run main loop. */
	core_setup(core_global, settings->window_title,
		settings->view_width, settings->view_height,
		settings->window_width, settings->window_height,
		args.windowed, 1000000000);
	vfs_run_callbacks();

#ifdef LOAD_SHARED
	/* Register game reload callback */
	vfs_register_callback(args.game, &load_game, 0);
#endif

	core_run(core_global);

	vfs_shutdown();
	return 0;
}
