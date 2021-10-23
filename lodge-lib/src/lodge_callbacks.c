#include "lodge_callbacks.h"

#include <string.h>

void lodge_callbacks_append(struct lodge_callbacks *callbacks, lodge_callback_func_t func, const void* data, size_t data_size)
{
	size_t data_offset = 0;

	for(size_t i = 0, count = callbacks->count; i < count; i++) {
		data_offset += callbacks->data_sizes[i];
	}

	const size_t index = callbacks->count++;

	callbacks->funcs[index] = func;
	callbacks->data_sizes[index] = data_size;
	callbacks->data_offsets[index] = data_offset;

	memcpy(&callbacks->data[data_offset], data, data_size);
}

void lodge_callbacks_run(const struct lodge_callbacks *callbacks)
{
	for(size_t i = 0, count = callbacks->count; i < count; i++) {
		const size_t data_offset = callbacks->data_offsets[i];
		callbacks->funcs[i](&callbacks->data[data_offset]);
	}
}

void lodge_callbacks_clear(struct lodge_callbacks *callbacks)
{
	callbacks->count = 0;
}