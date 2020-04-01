#ifndef _PYXEL_ASSET_H
#define _PYXEL_ASSET_H

#include "pyxel.h"
#include "atlas.h"
#include "animatedsprites.h"
#include "lodge_texture.h"

struct pyxel_asset {
	struct atlas	atlas;
	struct anim		anims[PYXEL_ANIMS_MAX];
	int				anims_count;
	char			anim_names[PYXEL_ANIMS_MAX][PYXEL_ANIM_NAME_MAX];
	lodge_texture_t	layers[PYXEL_LAYERS_MAX];
	int				layers_count;
	lodge_texture_t	layers_blended;
	char			layer_names[PYXEL_LAYERS_MAX][PYXEL_LAYER_NAME_MAX];
};

int					pyxel_asset_load(struct pyxel_asset *asset, void *data, size_t size);
void				pyxel_asset_free(struct pyxel_asset *asset);

struct anim*		pyxel_asset_get_anim(struct pyxel_asset *asset, const char *anim_name);
lodge_texture_t*	pyxel_asset_get_layer(struct pyxel_asset *asset, const char *layer_name);

#endif
