#ifndef _LODGE_H
#define _LODGE_H

#include "math4.h"

struct game_settings {
	int			window_width;
	int			window_height;
	const char	*window_title;
	int			view_width;
	int			view_height;
	vec3		sound_listener;
	float		sound_distance_max;
};

void lodge_start(struct game_settings* game_settings, int window_mode);

#endif
