#ifndef _ATLAS_H
#define _ATLAS_H

#include <stdio.h>

#include "log.h"

#define atlas_debug(...) debugf("Atlas", __VA_ARGS__)
#define atlas_error(...) errorf("Atlas", __VA_ARGS__)

#define ATLAS_OK		0
#define ATLAS_ERROR		-1

#define ATLAS_STR_MAX	256

struct atlas_frame {
	int		x;
	int		y;
	int		width;
	int		height;
	int		rotated;
	int		trimmed;
	char	name[ATLAS_STR_MAX];
#ifdef ATLAS_FATTY
	int		src_size_w;
	int		src_size_h;
	int		sprite_src_x;
	int		sprite_src_y;
	int		sprite_src_w;
	int		sprite_src_h;
#endif
};

/**
 * NOTE: Workaround for GCC Bug 53119 (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119),
 * the first member of the struct must not be an array if the zero initializer:
 *     struct example e = { 0 };
 * is to be used.
 */
struct atlas {
	int					width;
	int					height;
	char				image[ATLAS_STR_MAX];
	char				format[ATLAS_STR_MAX];
	int					frames_count;
	struct atlas_frame	*frames;
#ifdef ATLAS_FATTY
	char				scale[ATLAS_STR_MAX];
	char				generator[ATLAS_STR_MAX];
	char				generator_version[ATLAS_STR_MAX];
#endif
};

int		atlas_load(struct atlas *atlas, void *data, size_t data_len);
void	atlas_free(struct atlas *atlas);
void	atlas_print(struct atlas *atlas);
int		atlas_frame_index(struct atlas *atlas, const char *name);

#endif
