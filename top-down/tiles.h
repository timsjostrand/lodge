#ifndef _TILES_H
#define _TILES_H

#include "math4.h"
#include "graphics.h"
#include "atlas.h"
#include "animatedsprites.h"

typedef struct anim *(*tiles_get_data_at_pixel_t)(float x, float y, int tile_size, int grid_x_max, int grid_y_max);

struct tiles {
	int						tile_size;
	int						view_width;
	int						view_height;
	int						tiles_x;
	int						tiles_y;
	int						draw_tiles_x;
	int						draw_tiles_y;
	struct animatedsprites	*batcher;
	struct sprite			*draw_tiles;

	tiles_get_data_at_pixel_t get_data_at_pixel_fn;
};

void tiles_init(struct tiles *tiles, tiles_get_data_at_pixel_t get_data_at_pixel_fn, int tile_size,
		int view_width, int view_height, int tiles_x, int tiles_y);
void tiles_render(struct tiles *tiles, struct shader *s, struct graphics *g, GLuint tex, mat4 transform);
void tiles_think(struct tiles *tiles, vec2 view_offset, struct atlas *atlas, float dt);

#endif
