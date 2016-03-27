#ifndef _PYXEL_ASSET_H
#define _PYXEL_ASSET_H

#include <GLFW/glfw3.h>

#include "pyxel.h"
#include "atlas.h"
#include "animatedsprites.h"

struct pyxel_asset {
	struct atlas	atlas;
	struct anim		anims[PYXEL_ANIMS_MAX];
	int				anims_count;
	char			anim_names[PYXEL_ANIMS_MAX][PYXEL_ANIM_NAME_MAX];
	GLuint			layers[PYXEL_LAYERS_MAX];
	int				layers_count;
	GLuint			layers_blended;
	char			layer_names[PYXEL_LAYERS_MAX][PYXEL_LAYER_NAME_MAX];
};

int				pyxel_asset_load(struct pyxel_asset *asset, void *data, size_t size);
void			pyxel_asset_free(struct pyxel_asset *asset);

struct anim*	pyxel_asset_get_anim(struct pyxel_asset *asset, const char *anim_name);
GLuint*			pyxel_asset_get_layer(struct pyxel_asset *asset, const char *layer_name);

#endif
