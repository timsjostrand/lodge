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
#include "util_reload.h"
#include "lodge_image.h"

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

/**
 * FIXME: Super slow alpha blending.
 */
uint32_t pyxel_asset_blend_alpha(uint32_t top, uint32_t bottom)
{
#if 0
	uint8_t top_a = (top & 0xFF000000) >> 24;
	uint8_t top_b = (top & 0x00FF0000) >> 16;
	uint8_t top_g = (top & 0x0000FF00) >> 8;
	uint8_t top_r = (top & 0x000000FF);

	uint8_t bot_a = (bottom & 0xFF000000) >> 24;
	uint8_t bot_b = (bottom & 0x00FF0000) >> 16;
	uint8_t bot_g = (bottom & 0x0000FF00) >> 8;
	uint8_t bot_r = (bottom & 0x000000FF);

	uint8_t out_r = (bot_r * bot_a / 255) + (top_r * top_a * (255 - bot_a) / (255*255));
	uint8_t out_g = (bot_g * bot_a / 255) + (top_g * top_a * (255 - bot_a) / (255*255));
	uint8_t out_b = (bot_b * bot_a / 255) + (top_b * top_a * (255 - bot_a) / (255*255));
	uint8_t out_a = bot_a + (top_a * (255 - bot_a) / 255);

	return out_r + (out_g << 8) + (out_b << 16) + (out_a << 24);
#else
	uint8_t top_a = ((top & 0xFF000000) >> 24);
	uint8_t bot_a = ((bottom & 0xFF000000) >> 24);
	uint16_t out_alpha_factor = top_a * (255 - bot_a);
	uint8_t out_r = ((bottom & 0x000000FF) * bot_a / 255) + ((top & 0x000000FF) * out_alpha_factor / (255*255));
	uint8_t out_g = (((bottom & 0x0000FF00) >> 8) * bot_a / 255) + (((top & 0x0000FF00) >> 8) * out_alpha_factor / (255*255));
	uint8_t out_b = (((bottom & 0x00FF0000) >> 16) * bot_a / 255) + (((top & 0x00FF0000) >> 16) * out_alpha_factor / (255*255));
	uint8_t out_a = bot_a + (out_alpha_factor / 255);
	return out_r + (out_g << 8) + (out_b << 16) + (out_a << 24);
#endif
}

static int pyxel_asset_layers_blend(uint32_t **bufs, size_t *buf_sizes, int buf_count,
		void **out, size_t *out_size, int *out_width, int *out_height)
{
	uint32_t *final = NULL;
	struct lodge_image_out image_out;

	/* Unpack raw image data form all PNG files. */
	for(int i=0; i<buf_count; i++) {
		if(bufs[i] == NULL || buf_sizes[i] == 0) {
			continue;
		}

		pyxel_debug("Blending layer %d\n", i);

		uint8_t *buf = (uint8_t *) bufs[i];

		struct lodge_ret image_ret = lodge_image_new(&image_out, buf, buf_sizes[i]);
		if(!image_ret.success) {
			pyxel_error("images_blend(): could not load image %d: " STRVIEW_PRINTF_FMT "\n", i, STRVIEW_PRINTF_ARG(image_ret.message));
			goto bail;
		}

		/* Use the dimensions of the first processed layer to allocate memory for the
		 * final image. */
		if(final == NULL) {
			final = (uint32_t *) calloc(image_out.width * image_out.height, sizeof(uint32_t));
			if(final == NULL) {
				pyxel_error("Out of memory\n");
				goto bail;
			}
			(*out_size) = image_out.width * image_out.height * sizeof(uint32_t);
			(*out_width) = image_out.width;
			(*out_height) = image_out.height;
		}

		/* Blend color components to produce final image. */
		for(int x=0; x<image_out.width; x++) {
			for(int y=0; y<image_out.height; y++) {
				final[y * image_out.width + x] = pyxel_asset_blend_alpha(
					image_out.pixel_data[y * image_out.width + x],
					final[y * image_out.width + x]
				);
			}
		}

		lodge_image_free((uint8_t *) image_out.pixel_data);
	}

	(*out) = final;

	return PYXEL_OK;

bail:
	if(image_out.pixel_data != NULL) {
		image_free((uint8_t *) image_out.pixel_data);
	}

	if(final != NULL) {
		free(final);
	}

	return PYXEL_ERROR;
}

int pyxel_asset_load(struct pyxel_asset *asset, void *data, size_t data_len)
{
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
				(float)pyxel.anims[i].frame_duration);

		pyxel_debug("Parsed animation \"%s\" (%d => %d, %.0f ms)\n",
				asset->anim_names[i],
				asset->anims[i].frame_start,
				asset->anims[i].frame_count,
				asset->anims[i].frame_length);
	}

	/* Store layers here. */
	uint32_t *(tex_bufs)[PYXEL_LAYERS_MAX] = { NULL };
	size_t tex_sizes[PYXEL_LAYERS_MAX] = { 0 };

	/* Convert textures. */
	asset->layers_count = pyxel.layers_count;
	for(int i=0; i<pyxel.layers_count; i++) {
		/* Store name. */
		memcpy(asset->layer_names[i], pyxel.layers[i].name, PYXEL_LAYER_NAME_MAX);

		/* Texture file names are not equal to layer names! */
		char tex_filename[PYXEL_STR_MAX];
		snprintf((char *) &tex_filename, PYXEL_STR_MAX, "layer%d.png", i);

		/* Get size of texture file. */
		if(pyxel_archive_file_size(data, data_len, tex_filename, &tex_sizes[i]) != PYXEL_OK) {
			pyxel_error("Could not get size of texture \"%s\"\n", tex_filename);
			goto bail;
		}

		/* Allocate temporary buffer to extract texture to. */
		tex_bufs[i] = (uint32_t *) malloc(tex_sizes[i]);
		if(tex_bufs[i] == NULL) {
			goto bail;
		}
		/* Extract texture file. */
		if(pyxel_archive_extract(data, data_len, tex_filename, (char *) tex_bufs[i], tex_sizes[i]) != PYXEL_OK) {
			pyxel_error("Could not extract texture \"%s\"\n", tex_filename);
			goto bail;
		}
		pyxel_debug("Successfully extracted texture \"%s\" (%d bytes)\n", tex_filename, tex_sizes[i]);

		/* Upload texture to GPU. */
		util_reload_texture(tex_filename, tex_sizes[i], tex_bufs[i], &asset->layers[i]);
	}

	/* Combine (blend) layers. */
	void *blend_buf = NULL;
	int blend_width = 0;
	int blend_height = 0;
	size_t blend_size = 0;
	if(pyxel_asset_layers_blend(tex_bufs, tex_sizes, asset->layers_count, &blend_buf, &blend_size,
			&blend_width, &blend_height) != PYXEL_OK) {
		pyxel_error("Could not blend layers\n");
		goto bail;
	}
	util_reload_texture_pixels("layers_blended", blend_size, blend_buf,
			&asset->layers_blended, blend_width, blend_height);

	/* Clean up temporary texture memory. */
	for(int i=0; i<PYXEL_LAYERS_MAX; i++) {
		if(tex_bufs[i] != NULL) {
			free(tex_bufs[i]);
		}
	}

	if(blend_buf != NULL) {
		free(blend_buf);
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

	/* Clean up temporary texture memory. */
	for(int i=0; i<PYXEL_LAYERS_MAX; i++) {
		if(tex_bufs[i] != NULL) {
			free(tex_bufs[i]);
		}
	}

	if(blend_buf != NULL) {
		free(blend_buf);
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

	texture_free(asset->layers_blended);
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
	pyxel_warn("Could not find anim: %s\n", anim_name);
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
	pyxel_warn("Could not find layer: %s\n", layer_name);
	return NULL;
}
