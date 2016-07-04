#include "lodge.h"

void lodge_start(struct game_settings* settings, int window_mode)
{
	/* Sound setup */
	core_set_up_sound(&settings->sound_listener, settings->sound_distance_max);

	/* Initialize subsystems and run main loop. */
	core_setup(settings->window_title,
		settings->view_width, settings->view_height,
		settings->window_width, settings->window_height,
		window_mode, 1000000);
	vfs_run_callbacks();

	core_run();
}
