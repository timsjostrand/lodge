#ifndef _GAME_H
#define _GAME_H

#include <stdio.h>

struct graphics;
struct input;
struct GLFWwindow;
struct frames;
struct console;

typedef float vec3[3];

#ifdef _WIN32
#define EXPORT __declspec( dllexport ) 
#else
#define EXPORT
#endif

#ifdef ENABLE_SHARED
#define SHARED_SYMBOL EXPORT
#else
#define SHARED_SYMBOL 
#endif

#ifdef __cplusplus
extern "C"
{
#endif

SHARED_SYMBOL void game_init();

SHARED_SYMBOL void game_init_memory(void* memory);

SHARED_SYMBOL void game_assets_load();
SHARED_SYMBOL void game_assets_release();

SHARED_SYMBOL void game_think(struct graphics* g, float delta_time);
SHARED_SYMBOL void game_render(struct graphics* g, float delta_time);

SHARED_SYMBOL void game_key_callback(struct input* input, struct GLFWwindow* window, int key, int scancode, int action, int mods);
SHARED_SYMBOL void game_fps_callback(struct frames* f);

SHARED_SYMBOL void game_console_init(struct console* c);

SHARED_SYMBOL const int VIEW_WIDTH;
SHARED_SYMBOL const int VIEW_HEIGHT;

SHARED_SYMBOL vec3 sound_listener;

#ifdef __cplusplus
}
#endif

#endif
