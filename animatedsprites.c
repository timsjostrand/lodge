/**
* Animated sprites using spritebatch and atlas
*
* Author: Johan Yngman <johan.yngman@gmailcom>
*/

#include "animatedsprites.h"

#include <stdlib.h>

struct animatedsprites* animatedsprites_create()
{
	struct animatedsprites* as = (struct animatedsprites*)malloc(sizeof(struct animatedsprites));
	as->sprite_todraw_count = 0;
	spritebatch_create(&as->spritebatch);
	return as;
}

void animatedsprites_destroy(struct animatedsprites* animatedsprites)
{
	spritebatch_destroy(&animatedsprites->spritebatch);
	free(animatedsprites);
}

void animatedsprites_update(struct animatedsprites* animatedsprites, struct atlas* atlas, float delta_time)
{
	spritebatch_begin(&animatedsprites->spritebatch);

	for (int i = 0; i < animatedsprites->sprite_todraw_count; i++)
	{
		struct sprite* current_sprite = animatedsprites->sprites_todraw[i];

		/* Do not draw this sprite. */
		if (current_sprite->anim == NULL)
		{
			continue;
		}

		if (!current_sprite->state.done)
		{
			current_sprite->state.frame_time += delta_time;

			while (current_sprite->state.frame_time > current_sprite->anim->frame_length)
			{
				current_sprite->state.frame_time -= current_sprite->anim->frame_length;

				if (current_sprite->state.frame_current < current_sprite->anim->frame_start + current_sprite->anim->frame_count - 1)
				{
					current_sprite->state.frame_current++;
				}
				else if (current_sprite->anim->looping)
				{
					current_sprite->state.frame_current = current_sprite->anim->frame_start;
				}
				else
				{
					current_sprite->state.done = 1;
				}
			}
		}

		int index = current_sprite->state.frame_current;

		vec2 tex_pos;
		tex_pos[0] = (atlas->frames[index].x + 0.5f) / (float)atlas->width;
		tex_pos[1] = (atlas->frames[index].y + 0.5f) / (float)atlas->height;

		vec2 tex_bounds;
		tex_bounds[0] = (atlas->frames[index].width - 0.5f) / (float)atlas->width;
		tex_bounds[1] = (atlas->frames[index].height - 0.5f) / (float)atlas->height;

		vec2 scale;
		scale[0] = atlas->frames[index].width * current_sprite->scale[0];
		scale[1] = atlas->frames[index].height * current_sprite->scale[1];

		spritebatch_add(&animatedsprites->spritebatch, current_sprite->position, scale, tex_pos, tex_bounds);
	}

	spritebatch_end(&animatedsprites->spritebatch);
}

void animatedsprites_sort(struct animatedsprites* animatedsprites, spritebatch_sort_fn sorting_function)
{
	spritebatch_sort(&animatedsprites->spritebatch, sorting_function);
}

void animatedsprites_render(struct animatedsprites* animatedsprites, struct shader *s, struct graphics *g, GLuint tex, mat4 transform)
{
	spritebatch_render(&animatedsprites->spritebatch, s, g, tex, transform);
}

void animatedsprites_add(struct animatedsprites* animatedsprites, struct sprite* sprite)
{
	animatedsprites->sprites_todraw[animatedsprites->sprite_todraw_count] = sprite;
	animatedsprites->sprite_todraw_count++;
}

void animatedsprites_clear(struct animatedsprites* animatedsprites)
{
	animatedsprites->sprite_todraw_count = 0;
}

void animatedsprites_playanimation(struct sprite* sprite, struct anim* anim)
{
	/* Sanity check */
	/* NOTE: anim == NULL is legal! */
	if(sprite == NULL) {
		return;
	}

	sprite->anim = anim;

	/* Reset animation state. */
	sprite->state.done = 0;
	sprite->state.frame_current = anim != NULL ? anim->frame_start : 0;
	sprite->state.frame_time = 0;
}

void animatedsprites_setanim(struct anim* anim, int looping, int frame_start, int frame_count, float frame_length)
{
	anim->looping = looping;
	anim->frame_start = frame_start;
	anim->frame_count = frame_count;
	anim->frame_length = frame_length;
}
