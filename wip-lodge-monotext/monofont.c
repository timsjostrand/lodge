#include "monofont.h"

struct monofont monofont_make(lodge_texture_t atlas_texture, int32_t atlas_width, int32_t atlas_height, int32_t letter_width, int32_t letter_height, int32_t letter_spacing_x, int32_t letter_spacing_y)
{
	return (struct monofont) {
		.atlas_texture = atlas_texture,
		.letter_width = letter_width,
		.letter_height = letter_height,
		.letter_spacing_x = letter_spacing_x,
		.letter_spacing_y = letter_spacing_y,
		.width = atlas_width,
		.height = atlas_height,
		.grids_x = atlas_width / letter_width,
		.grids_y = atlas_height / letter_height,
	};
}

bool monofont_get_atlas_quad_01(const struct monofont *font, const char c, float *tx_out, float *ty_out, float *tw_out, float *th_out)
{
	char letter = c - MONOFONT_START_CHAR;

	/* Invalid char. */
	if(letter > c) {
		return false;
	}

	int grid_y = letter / font->grids_x;
	int grid_x = (letter - (grid_y * font->grids_x)) % font->grids_x;

	/* Map coordinates to [0,1]. */
	*(tx_out) = grid_x / (float)font->grids_x;
	*(ty_out) = grid_y / (float)font->grids_y;
	*(tw_out) = font->letter_width / (float)font->width;
	*(th_out) = font->letter_height / (float)font->height;

	return true;
}
