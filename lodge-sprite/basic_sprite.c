#include "basic_sprite.h"

#include "lodge.h"
#include "drawable.h"

// TODO(TS): rename sprite_make, return by value
void sprite_init(struct basic_sprite *sprite, int type, float x, float y, float z,
	float w, float h, const vec4 color, float rotation, lodge_texture_t texture)
{
	sprite->type = type;
	sprite->pos = vec4_make(x, y, z, 0.0f);
	sprite->scale = vec4_make(w, h, 1.0f, 1.0f);
	sprite->color = vec4_make(xyzw_of(color));
	sprite->rotation = rotation;
	sprite->texture = texture;
}

void sprite_render(struct basic_sprite *sprite, struct shader *s, mat4 projection, struct lodge_renderer *renderer)
{
	// Position, rotation and scale
	mat4 transform_position = mat4_translation(xyz_of(sprite->pos));
	mat4 transform_scale = mat4_scaling(xyz_of(sprite->scale));
	mat4 transform_rotation = mat4_rotation_z(sprite->rotation);
	mat4 transform_final = mat4_mult(transform_position, transform_rotation);
	transform_final = mat4_mult(transform_final, transform_scale);

	const struct mvp mvp = {
		.model = transform_final,
		.view = mat4_identity(),
		//.projection = mat4_ortho(0, g->view_width, g->view_height, 0, -1, 1)
		.projection = projection
	};

	// TODO(TS): port binding shader, texture
	drawable_render(lodge_renderer_get_unit_rect(renderer));

#if 0
	drawable_render_detailed(DRAW_MODE_TRIANGLES,
		,
		VBO_QUAD_VERTEX_COUNT,
		sprite->texture,
		sprite->color,
		s,
		mvp);
#endif
}