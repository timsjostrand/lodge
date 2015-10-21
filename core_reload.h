#ifndef _CORE_RELOAD_H
#define _CORE_RELOAD_H

void core_reload_sound(const char *filename, unsigned int size, void *data, void *userdata);
void core_reload_shader(const char *filename, unsigned int size, void *data, void *userdata);
void core_reload_atlas(const char *filename, unsigned int size, void *data, void *userdata);
void core_reload_texture(const char *filename, unsigned int size, void *data, void* userdata);
void core_reload_console_conf(const char *filename, unsigned int size, void *data, void *userdata);

#endif
