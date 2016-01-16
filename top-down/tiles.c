/**
 * Tile renderer for top-down games.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "tiles.h"
#include "log.h"
#include "math4.h"
#include "animatedsprites.h"

static struct sprite* tiles_get_draw_tile(struct sprite *draw_tiles, int x, int y, int x_max, int y_max)
{
	if((y * x_max + x) >= (x_max*y_max)) {
		return NULL;
	}
	return &draw_tiles[y * x_max + x];
}

/**
 * Creates a new batched tile renderer in the supplied 'tiles' struct.
 *
 * The renderer will look up tiles in the 'data' 2D array of tiles_x and
 * tiles_y dimensions.
 *
 * It will also create enough sprites ("draw tiles") to cover the view with
 * the specified tile_size. These draw tiles will be populated from the 'data'
 * array depending on the current viewport.
 *
 * @param tiles			Where to initialize the tile renderer.
 * @param data			2D array of tiles data (the animation for each tile).
 * @param tile_size		The width and height of each tile.
 * @param view_width	The width of the current view.
 * @param view_height	The height of the current view.
 * @param tiles_x		The width of the data 2D array.
 * @param tiles_y		The height of the data 2D array.
 */
void tiles_init(struct tiles *tiles, tiles_get_data_at_pixel_t get_data_at_pixel_fn, int tile_size,
		int view_width, int view_height, int tiles_x, int tiles_y)
{
	tiles->tile_size = tile_size;
	tiles->view_width = view_width;
	tiles->view_height = view_height;
	tiles->tiles_x = tiles_x;
	tiles->tiles_y = tiles_y;
	tiles->get_data_at_pixel_fn = get_data_at_pixel_fn;
	tiles->draw_tiles_x = view_width/tile_size + 3;
	tiles->draw_tiles_y = view_height/tile_size + 3;

	tiles->draw_tiles = (struct sprite *) calloc(tiles->draw_tiles_x * tiles->draw_tiles_y, sizeof(struct sprite));
	memset(tiles->draw_tiles, 0, tiles->draw_tiles_x * tiles->draw_tiles_y * sizeof(struct sprite));
	tiles->batcher = animatedsprites_create();
}

void tiles_free(struct tiles *tiles)
{
	free(tiles->draw_tiles);
	animatedsprites_destroy(tiles->batcher);
}

/**
 * Updates the currently visible tiles based on the offset and the view
 * width/height specified in tiles_init().
 *
 * @param tiles			The tiles struct.
 * @param view_offset	The current offset from origo for this viewport.
 * @param atlas			The atlas to draw from.
 * @param dt			Delta time.
 */
void tiles_think(struct tiles *tiles, vec2 view_offset, struct atlas *atlas, float dt)
{
	animatedsprites_clear(tiles->batcher);

	/* Update draw tiles */
	for(int x = 0; x < tiles->draw_tiles_x; x++) {
		for(int y = 0; y < tiles->draw_tiles_y; y++) {
			struct sprite *draw_tile = tiles_get_draw_tile(tiles->draw_tiles, x, y,
					tiles->draw_tiles_x, tiles->draw_tiles_y);

			if(draw_tile == NULL) {
				continue;
			}

			animatedsprites_add(tiles->batcher, draw_tile);

			float grid_x = (x-1) * tiles->tile_size + tiles->tile_size/2.0f - fmod(view_offset[0], tiles->tile_size);
			float grid_y = (y-1) * tiles->tile_size + tiles->tile_size/2.0f - fmod(view_offset[1], tiles->tile_size);

			/* Get tile at coordinate. */
			struct anim *type = tiles->get_data_at_pixel_fn(view_offset[0] + grid_x,
															view_offset[1] + grid_y,
															tiles->tile_size, tiles->tiles_x, tiles->tiles_y);

			/* Set correct animation. */
			if(draw_tile->anim != type) {
				animatedsprites_playanimation(draw_tile, type);
			}

			set2f(draw_tile->scale, 1.0f, 1.0f);

			/* Move within the viewport. */
			set3f(draw_tile->position, grid_x, grid_y, 0);
		}
	}

	animatedsprites_update(tiles->batcher, atlas, dt);
}

void tiles_render(struct tiles *tiles, struct shader *s, struct graphics *g, GLuint tex, mat4 transform)
{
	animatedsprites_render(tiles->batcher, s, g, tex, transform);
}
