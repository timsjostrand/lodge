#include "water.h"

#include "color.h"
#include "lodge_sampler.h"
#include "lodge_texture.h"

void water_init(struct water *water, struct water_textures textures, vec3 *pos, vec3 *scale, vec3 *offset)
{
	water->textures = textures;
	water->pos = pos;
	water->scale = scale;
	water->offset = offset;
	water->wave_scale = vec3_make(100.0f, 100.0f, 0.1f);

	vec2 origin = { -0.5f, -0.5f };
	vec2 size = { 1.0f, 1.0f };
	water->drawable = drawable_make_plane_subdivided(origin, size, 1, 1);

	water->samplers.linear_repeat = lodge_sampler_make_properties((struct lodge_sampler_properties) {
		.min_filter = MIN_FILTER_LINEAR,
		.mag_filter = MAG_FILTER_LINEAR,
		.wrap_x = WRAP_REPEAT,
		.wrap_y = WRAP_REPEAT,
		.wrap_z = WRAP_REPEAT,
	});
	water->samplers.linear_clamp = lodge_sampler_make_properties((struct lodge_sampler_properties) {
		.min_filter = MIN_FILTER_LINEAR,
		.mag_filter = MAG_FILTER_LINEAR,
		.wrap_x = WRAP_CLAMP_TO_EDGE,
		.wrap_y = WRAP_CLAMP_TO_EDGE,
		.wrap_z = WRAP_CLAMP_TO_EDGE,
	});
}

void water_render(struct water *water, lodge_shader_t shader, const struct water_render_params params)
{
	lodge_renderer_bind_shader(shader);

	lodge_renderer_set_constant_float(shader, strview_static("time"), params.time);
	lodge_renderer_set_constant_vec3(shader, strview_static("camera_pos"), params.camera_pos);
	lodge_renderer_set_constant_vec3(shader, strview_static("terrain_scale"), params.terrain_scale);
	lodge_renderer_set_constant_vec3(shader, strview_static("sun_dir"), params.sun_dir);

	vec3 pos = vec3_add(*water->pos, *water->offset);
	mat4 model = mat4_translation(xyz_of(pos));
	model = mat4_scale(model, water->scale->x, water->scale->y, water->scale->z);

	if(water->textures.heightmap) {
		lodge_renderer_bind_texture_unit_2d(0, water->textures.heightmap, water->samplers.linear_clamp);
	}

	if(water->textures.cubemap) {
		lodge_renderer_bind_texture_unit_cube_map(1, water->textures.cubemap, water->samplers.linear_repeat);
	}

	lodge_renderer_bind_texture_unit_2d(3, params.depth_texture, water->samplers.linear_repeat);
	lodge_renderer_bind_texture_unit_2d(4, params.color_texture, water->samplers.linear_repeat);

	if(water->textures.water_foam) {
		lodge_renderer_bind_texture_unit_2d(2, water->textures.water_foam, water->samplers.linear_repeat);
	}

	if(water->textures.water_normals[0]) {
		lodge_renderer_bind_texture_unit_2d(5, water->textures.water_normals[0], water->samplers.linear_repeat);
	}

	if(water->textures.water_normals[1]) {
		lodge_renderer_bind_texture_unit_2d(6, water->textures.water_normals[1], water->samplers.linear_repeat);
	}

	lodge_renderer_set_constant_vec3(shader, strview_static("scale"), *water->scale);
	lodge_renderer_set_constant_vec3(shader, strview_static("wave_scale"), water->wave_scale);

	struct mvp mvp = {
		.model = model,
		.view = params.view,
		.projection = params.perspective
	};

	lodge_renderer_set_constant_mvp(shader, &mvp);

	drawable_render(&water->drawable);

	// FIXME(TS): hack
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, *water->textures.heightmap);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
