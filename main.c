#include "game.h"

#include "log.h"

#ifdef _WIN32
#include "windows.h"
#else
#include <dlfcn.h>
#endif

#include "vfs.h"
#include "assets.h"

#include "core.h"
#include "core_argv.h"
#include "core_reload.h"

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

void free_shared_library(void* library)
{
#ifdef _WIN32
	return FreeLibrary((HINSTANCE)library);
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

	game_library = load_shared_library(filename);
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
			core_set_asset_callbacks(game_assets_load_fn, game_init_fn, game_assets_release_fn);
			core_set_key_callback(game_key_callback_fn);
			core_set_fps_callback(game_fps_callback_fn);
			core_set_console_init_callback(game_console_init_fn);
			core_set_think_callback(game_think_fn);
			core_set_render_callback(game_render_fn);
			core_set_init_memory_callback(game_init_memory_fn);

			if (!first_load)
			{
				game_assets_load_fn();
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

	/* Sound setup */
	set3f(sound_listener, VIEW_WIDTH / 2.0f, VIEW_HEIGHT / 2.0f, 0);
	vec3 sound_audible_max = { VIEW_WIDTH, VIEW_HEIGHT, 0.0f };
	float sound_distance_max = distance3f(sound_listener, sound_audible_max);
	core_set_up_sound(&sound_listener, sound_distance_max);

#ifdef ENABLE_SHARED
	/* Load game library */
	size_t filesize;
	void* lib = vfs_get_file(args.game, &filesize);
	load_game(args.game, filesize, lib, 0);
	first_load = 0;
#else
	core_set_think_callback(&game_think);
	core_set_render_callback(&game_render);
	core_set_asset_callbacks(&game_assets_load, &game_init, &game_assets_release);
	core_set_key_callback(&game_key_callback);
	core_set_fps_callback(&game_fps_callback);
	core_set_init_memory_callback(&game_init_memory);
	core_set_up_console(&game_console_init, &assets.shaders.basic_shader);
#endif

	/* Initialize subsystems and run main loop. */
	core_setup("glpong", VIEW_WIDTH, VIEW_HEIGHT, VIEW_WIDTH, VIEW_HEIGHT, args.windowed, 1000000000);
	vfs_run_callbacks();

#ifdef ENABLE_SHARED
	/* Register game reload callback */
	vfs_register_callback(args.game, &load_game, 0);
#endif

	core_run();

	vfs_shutdown();
	return 0;
}
