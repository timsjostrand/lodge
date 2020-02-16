#include "lodge_draw_call.h"

struct lodge_texture_slot lodge_texture_slot_make(strview_t name, struct lodge_sampler sampler, struct lodge_texture texture)
{
	return (struct lodge_texture) {
		.name = name,
		.sampler = sampler,
		.texture = texture
	};
}

struct lodge_uniform_make(strview_t name, const void *data, struct lodge_uniform_type)
{
	return (struct lodge_uniform) {
		.name = name,
		.data = data,
		.type = type
	};
}
