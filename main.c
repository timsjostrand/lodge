#include "game.h"

#include "vfs.h"
#include "assets.h"

#include "core.h"
#include "core_argv.h"
#include "core_reload.h"

int main(int argc, char **argv)
{
	/* Parse command line arguments. */
	struct core_argv args = { 0 };
	core_argv_parse(&args, argc, argv);

	set3f(sound_listener, VIEW_WIDTH / 2.0f, VIEW_HEIGHT / 2.0f, 0);
	vec3 sound_audible_max = { VIEW_WIDTH, VIEW_HEIGHT, 0.0f };
	float sound_distance_max = distance3f(sound_listener, sound_audible_max);

	/* Hook up core. */
	core_set_asset_callbacks(&game_assets_load, &game_init, &game_assets_release);
	core_set_key_callback(&game_key_callback);
	core_set_fps_callback(&game_fps_callback);
	core_set_up_console(&game_console_init, &assets.shaders.basic_shader);
	core_set_up_sound(&sound_listener, sound_distance_max);

	/* Initialize subsystems and run main loop. */
	core_run("glpong", VIEW_WIDTH, VIEW_HEIGHT, VIEW_WIDTH, VIEW_HEIGHT,
		args.windowed, args.mount, &game_think, &game_render);

	return 0;
}
