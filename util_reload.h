#ifndef _CORE_RELOAD_H
#define _CORE_RELOAD_H

void util_reload_sound(const char *filename, unsigned int size, void *data, void *userdata);
void util_reload_shader(const char *filename, unsigned int size, void *data, void *userdata);
void util_reload_atlas(const char *filename, unsigned int size, void *data, void *userdata);
void util_reload_texture(const char *filename, unsigned int size, void *data, void* userdata);
void util_reload_texture_pixels(const char *filename, unsigned int size, void *data, void* userdata, int width, int height);
void util_reload_console_conf(const char *filename, unsigned int size, void *data, void *userdata);
void util_reload_pyxel_asset(const char *filename, unsigned int size, void *data, void *userdata);

#endif
