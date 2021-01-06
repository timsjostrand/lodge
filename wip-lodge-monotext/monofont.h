#ifndef _MONOFONT_H
#define _MONOFONT_H

#define MONOFONT_START_CHAR	32

#include "lodge_platform.h"

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

struct monofont
{
	const char		*name;
	lodge_texture_t	atlas_texture;
	int32_t			width;
	int32_t			height;
	int32_t			letter_width;
	int32_t			letter_height;
	int32_t			letter_spacing_x;
	int32_t			letter_spacing_y;
	int32_t			grids_x;
	int32_t			grids_y;
};

struct monofont	monofont_make(lodge_texture_t atlas_texture, int32_t atlas_width, int32_t atlas_height, int32_t letter_width, int32_t letter_height, int32_t letter_spacing_x, int32_t letter_spacing_y);
bool			monofont_get_atlas_quad_01(const struct monofont *font, const char c, float *tx_out, float *ty_out, float *tw_out, float *th_out);

#endif
