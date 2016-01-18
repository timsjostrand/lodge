/**
 * Utilies that converts the contents of a .pyxel file parsed from 'pyxel.c'
 * into assets the engine can use (atlas, animations, textures).
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>.
 */

#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "pyxel_asset.h"
#include "texture.h"
#include "core_reload.h"

static int pyxel_asset_init_atlas(struct atlas *atlas, struct pyxel *pyxel)
{
	/* Parse atlas meta information. */
	atlas->width = pyxel->width;
	atlas->height = pyxel->height;
	memset(atlas->image, 0, sizeof(atlas->image));		/* Ignored. */
	memset(atlas->format, 0, sizeof(atlas->format));	/* Ignored. */

	int tile_col_count = atlas->width / pyxel->tile_width;
	int tile_row_count = atlas->height / pyxel->tile_height;
	atlas->frames_count = tile_row_count * tile_col_count;

	/* Allocate memory for frames (sprites). */
	atlas->frames = (struct atlas_frame *) calloc(atlas->frames_count, sizeof(struct atlas_frame));
	if(atlas->frames == NULL) {
		return PYXEL_ERROR;
	}

	/* Calculate frames from sprite information. */
	for(int y=0; y<tile_row_count; y++) {
		for(int x=0; x<tile_col_count; x++) {
			struct atlas_frame *f = &atlas->frames[y * tile_col_count + x];

			f->x = x * pyxel->tile_width;
			f->y = y * pyxel->tile_height;
			f->width = pyxel->tile_width;
			f->height = pyxel->tile_height;
			f->rotated = 0;
			f->trimmed = 0;
			memset(f->name, 0, sizeof(f->name));
		}
	}

	return PYXEL_OK;
}

int pyxel_asset_load(struct pyxel_asset *asset, void *data, size_t data_len)
{
	char *tex_buf = NULL;

	/* Parse .pyxel-file. */
	struct pyxel pyxel;
	PYXEL_OK_OR_BAIL(pyxel_load(&pyxel, data, data_len));

	/* Convert anims. */
	asset->anims_count = pyxel.anims_count;
	for(int i=0; i<pyxel.anims_count; i++) {
		/* Store name. */
		memcpy(asset->anim_names[i], pyxel.anims[i].name, PYXEL_ANIM_NAME_MAX);

		animatedsprites_setanim(&asset->anims[i],
				1,
				pyxel.anims[i].base_tile,
				pyxel.anims[i].length,
				pyxel.anims[i].frame_duration);

		pyxel_debug("Parsed animation \"%s\" (%d => %d, %.0f ms)\n",
				asset->anim_names[i],
				asset->anims[i].frame_start,
				asset->anims[i].frame_count,
				asset->anims[i].frame_length);
	}

	/* Convert textures. */
	asset->layers_count = pyxel.layers_count;
	for(int i=0; i<pyxel.layers_count; i++) {
		/* Store name. */
		memcpy(asset->layer_names[i], pyxel.layers[i].name, PYXEL_LAYER_NAME_MAX);

		/* Texture file names are not equal to layer names! */
		char tex_filename[PYXEL_STR_MAX];
		snprintf(&tex_filename, PYXEL_STR_MAX, "layer%d.png", i);

		/* Get size of texture file. */
		size_t tex_size;
		if(pyxel_archive_file_size(data, data_len, tex_filename, &tex_size) != PYXEL_OK) {
			pyxel_error("Could not get size of texture \"%s\"\n", tex_filename);
			goto bail;
		}

		/* Allocate temporary buffer to extract texture to. */
		tex_buf = (char *) malloc(tex_size);
		if(tex_buf == NULL) {
			goto bail;
		}
		/* Extract texture file. */
		if(pyxel_archive_extract(data, data_len, tex_filename, tex_buf, tex_size) != PYXEL_OK) {
			pyxel_error("Could not extract texture \"%s\"\n", tex_filename);
			goto bail;
		}
		pyxel_debug("Successfully extracted texture \"%s\" (%d bytes)\n", tex_filename, tex_size);

		/* Upload texture to GPU. */
		core_reload_texture(tex_filename, tex_size, tex_buf, &asset->layers[i]);

		/* Clean up temporary texture memory. */
		if(tex_buf != NULL) {
			free(tex_buf);
		}
	}

	/* Create atlas from meta information. */
	if(pyxel_asset_init_atlas(&asset->atlas, &pyxel) != PYXEL_OK) {
		pyxel_error("Could not parse atlas information\n");
		goto bail;
	}
	atlas_print(&asset->atlas);

	//pyxel_free(&pyxel);
	return PYXEL_OK;

bail:
	//pyxel_free(&pyxel);

	if(tex_buf != NULL) {
		free(tex_buf);
	}

	pyxel_asset_free(asset);

	return PYXEL_ERROR;
}

void pyxel_asset_free(struct pyxel_asset *asset)
{
	atlas_free(&asset->atlas);

	for(int i=0; i<PYXEL_LAYERS_MAX; i++) {
		texture_free(asset->layers[i]);
	}
}

/**
 * Get an animation by name from the pyxel asset.
 *
 * To get by index, use asset->anims[index] instead.
 */
struct anim* pyxel_asset_get_anim(struct pyxel_asset *asset, const char *anim_name)
{
	for(int i=0; i<asset->anims_count; i++) {
		if(strcmp(asset->anim_names[i], anim_name) == 0) {
			return &asset->anims[i];
		}
	}
	return NULL;
}

/**
 * Get a layer by name from the pyxel asset. To get by index, use
 * asset->layers[index] instead.
 */
GLuint* pyxel_asset_get_layer(struct pyxel_asset *asset, const char *layer_name)
{
	for(int i=0; i<asset->layers_count; i++) {
		if(strcmp(asset->layer_names[i], layer_name) == 0) {
			return &asset->layers[i];
		}
	}
	return NULL;
}
