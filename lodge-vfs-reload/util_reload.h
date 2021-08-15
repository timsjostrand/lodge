#ifndef _CORE_RELOAD_H
#define _CORE_RELOAD_H

#include "strview.h"

struct lodge_vfs;

void util_reload_sound(struct lodge_vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata);

#if 0
void util_reload_shader(struct lodge_vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata);
#endif

void util_reload_texture(struct lodge_vfs *vfs, strview_t filename, size_t size, const void *data, void* userdata);
void util_reload_texture_pixels(struct lodge_vfs *vfs, strview_t filename, size_t size, const void *data, void* userdata, int width, int height);

#ifdef ENABLE_LODGE_CONSOLE
void util_reload_console_conf(struct lodge_vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata);
#endif

#ifdef ENABLE_LODGE_ASSET_ATLAS
void util_reload_atlas(struct lodge_vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata);
#endif

#ifdef ENABLE_LODGE_ASSET_PYXEL
void util_reload_pyxel_asset(struct lodge_vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata);
#endif

#endif
