#ifndef _GAME_H
#define _GAME_H

#include <stdio.h>

#include "vfs.h"

struct graphics;
struct input;
struct GLFWwindow;
struct frames;
struct console;
struct core;
struct assets;

typedef float vec3[3];

struct shared_memory
{
	void* game_memory;
	struct core* core;
	struct assets* assets;
	struct vfs* vfs;
	struct input* input;
};

//#define LOAD_SHARED

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

#ifdef __cplusplus
extern "C"
{
#endif

struct game_settings {
	int			window_width;
	int			window_height;
	const char	*window_title;
	int			view_width;
	int			view_height;
	vec3		sound_listener;
	float		sound_distance_max;
};

SHARED_SYMBOL void game_init();

SHARED_SYMBOL void game_init_memory(struct shared_memory* shared_memory, int reload);

SHARED_SYMBOL void game_assets_load();
SHARED_SYMBOL void game_assets_release();

SHARED_SYMBOL void game_think(struct core* core, struct graphics* g, float delta_time);
SHARED_SYMBOL void game_render(struct core* core, struct graphics* g, float delta_time);

SHARED_SYMBOL void game_key_callback(struct core* core, struct input* input, struct GLFWwindow* window, int key, int scancode, int action, int mods);
SHARED_SYMBOL void game_mousebutton_callback(struct core* core, struct GLFWwindow *window, int button, int action, int mods);
SHARED_SYMBOL void game_fps_callback(struct frames* f);

SHARED_SYMBOL void game_console_init(struct console* c);

SHARED_SYMBOL struct game_settings* game_get_settings();

#ifdef __cplusplus
}
#endif

#endif
