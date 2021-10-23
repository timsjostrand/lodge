#ifndef _LODGE_CALLBACKS_H
#define _LODGE_CALLBACKS_H

#include <stddef.h>

typedef void (*lodge_callback_func_t)(const void* userdata);

#define LODGE_CALLBACKS_MAX				256
#define LODGE_CALLBACKS_USERDATA_MAX	1024

struct lodge_callbacks
{
	size_t                  count;
	lodge_callback_func_t   funcs[LODGE_CALLBACKS_MAX];
	size_t                  data_offsets[LODGE_CALLBACKS_MAX];
	size_t                  data_sizes[LODGE_CALLBACKS_MAX];
	char                    data[LODGE_CALLBACKS_MAX * LODGE_CALLBACKS_USERDATA_MAX];
};

void lodge_callbacks_run(const struct lodge_callbacks *callbacks);
void lodge_callbacks_append(struct lodge_callbacks *callbacks, lodge_callback_func_t func, const void* data, size_t data_size);
void lodge_callbacks_clear(struct lodge_callbacks *callbacks);

#endif