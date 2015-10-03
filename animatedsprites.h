#ifndef _ANIMATEDSPRITES_H
#define _ANIMATEDSPRITES_H

#include "graphics.h"
#include "spritebatch.h"
#include "atlas.h"

#define ANIMATEDSPRITES_MAX_SPRITES 10000

typedef float GLfloat;
typedef unsigned int GLuint;
typedef float mat4[16];
typedef float vec3[3];
typedef float vec2[2];

struct sprite
{
	vec3 position;
	vec2 scale;

	int looping;
	int done;

	int frame_start;
	int frame_count;
	int frame_current;

	float frame_length;
	float frame_time;
};

struct animatedsprites
{
	struct sprite* sprites_todraw[ANIMATEDSPRITES_MAX_SPRITES];
	struct spritebatch spritebatch;
	unsigned int sprite_todraw_count;
};

struct animatedsprites* animatedsprites_create();
void animatedsprites_destroy(struct animatedsprites* animatedsprites);

void animatedsprites_playanimation(struct sprite* sprite, int frame_start, int frame_count, float frame_length, int looping);
void animatedsprites_update(struct animatedsprites* animatedsprites, struct atlas* atlas, float delta_time);
void animatedsprites_render(struct animatedsprites* animatedsprites, struct shader *s, struct graphics *g, GLuint tex);

void animatedsprites_add(struct animatedsprites* animatedsprites, struct sprite* sprite);
void animatedsprites_clear(struct animatedsprites* animatedsprites);

#endif //_ANIMATEDSPRITES_H
