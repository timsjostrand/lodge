#include "basic_sprite.h"

#include <GL/glew.h>
#include "drawable.h"

/**** DEPRECATED STUFF ****/

void sprite_render(struct basic_sprite *sprite, struct shader *s, struct graphics *g)
{
	// Position, rotation and scale
	mat4 transform_position = mat4_translation(xyz(sprite->pos));
	mat4 transform_scale = mat4_scaling(xyz(sprite->scale));
	mat4 transform_rotation = mat4_rotation_z(sprite->rotation);
	mat4 transform_final = mat4_mult(transform_position, transform_rotation);
	transform_final = mat4_mult(transform_final, transform_scale);

	const struct mvp mvp = {
		.model = transform_final,
		.view = mat4_identity(),
		.projection = mat4_ortho(0, g->view_width, g->view_height, 0, -1, 1)
	};

	drawable_render_detailed(GL_TRIANGLES,
		g->vbo_rect,
		VBO_QUAD_VERTEX_COUNT,
		g->vao_rect,
		sprite->texture,
		sprite->color,
		s,
		mvp);
}

void sprite_init(struct basic_sprite *sprite, int type, float x, float y, float z,
	float w, float h, const vec4 color, float rotation, GLuint *texture)
{
	sprite->type = type;
	sprite->pos = vec4_make(x, y, z, 0.0f);
	sprite->scale = vec4_make(w, h, 1.0f, 1.0f);
	sprite->color = vec4_make(xyzw(color));
	sprite->rotation = rotation;
	sprite->texture = texture;
}
