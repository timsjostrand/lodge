#ifndef _PYXEL_H
#define _PYXEL_H

#include <stdio.h>

#include "log.h"

#define pyxel_debug(...) debugf("PyxelFormat", __VA_ARGS__)
#define pyxel_error(...) errorf("PyxelFormat", __VA_ARGS__)

#define PYXEL_OK		0
#define PYXEL_ERROR		-1

#define PYXEL_DOC_DATA_MAX		(64 * 1024)
#define PYXEL_LAYERS_MAX		8
#define PYXEL_ANIMS_MAX			256
#define PYXEL_STR_MAX			256
#define PYXEL_LAYER_NAME_MAX	PYXEL_STR_MAX
#define PYXEL_ANIM_NAME_MAX		PYXEL_STR_MAX

#define PYXEL_OK_OR_BAIL(x) if(x != PYXEL_OK) goto bail

struct pyxel_layer {
	char				name[PYXEL_LAYER_NAME_MAX];
};

struct pyxel_anim {
	char				name[PYXEL_ANIM_NAME_MAX];
	int					base_tile;
	int					length;
	int					frame_duration;
};

struct pyxel {
	int					tile_width;
	int					tile_height;
	int					num_layers;
	int					width;
	int					height;
	struct pyxel_anim	anims[PYXEL_ANIMS_MAX];
	int					anims_count;
	struct pyxel_layer	layers[PYXEL_LAYERS_MAX];
	int					layers_count;
};

int		pyxel_load(struct pyxel *pyxel, void *data, size_t data_len);
#if 0
void	pyxel_free(struct pyxel *pyxel);
#endif

int		pyxel_archive_file_size(void *data, const size_t data_len, const char *filename, size_t *size);
int		pyxel_archive_extract(void *data, size_t data_len, char *filename, char *dst, size_t dst_len);

#endif
