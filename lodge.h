#ifndef _LODGE_H
#define _LODGE_H

#ifdef _WIN32
#define EXPORT __declspec( dllexport )
#define IMPORT __declspec( dllimport )
#else
#define EXPORT
#define IMPORT
#endif

#ifdef ENABLE_SHARED
#define SHARED_SYMBOL EXPORT
#else

#ifdef LOAD_SHARED
#define SHARED_SYMBOL IMPORT
#else
#define SHARED_SYMBOL
#endif

#endif

#include "math4.h"

struct lodge_settings {
	int			window_width;
	int			window_height;
	const char	*window_title;
	int			view_width;
	int			view_height;
	vec3		sound_listener;
	float		sound_distance_max;
};

EXPORT void lodge_start(struct lodge_settings* lodge_settings, int window_mode);

#endif
