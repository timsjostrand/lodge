/**
 * Loader helpers.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include "util_reload.h"

#include "lodge_vfs.h"
#include "sound.h"
#include "array.h"

#include "lodge_image.h"
#include "lodge_texture.h"
#include "lodge_shader.h"

#ifdef ENABLE_LODGE_CONSOLE
#include "console.h"
#endif

#ifdef ENABLE_LODGE_ASSET_PYXEL
#include "pyxel_asset.h"
#endif

#ifdef ENABLE_LODGE_ASSET_ATLAS
#include "atlas.h"
#endif

#include "lodge_opengl.h"

#include <string.h>

#define reload_debug(FMT, ...) debugf("Reload", FMT, __VA_ARGS__)
#define reload_error(FMT, ...) errorf("Reload", FMT, __VA_ARGS__)

void util_reload_sound(struct lodge_vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata)
{
	if(size == 0) {
		sound_debug("Skipped reload of `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
		return;
	}

	sound_buf_t tmp = 0;
	sound_buf_t *dst = (sound_buf_t *) userdata;

	/* Reload sound. */
	if(sound_buf_load_vorbis(&tmp, data, size) != SOUND_OK) {
		sound_error("Could not load `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
	} else {
		/* Release current sound (if any). */
		sound_buf_free((*dst));

		/* Assign new sound (only if parsing was OK). */
		(*dst) = tmp;
	}
}

// FIXME(TS): strbuf and strview?
static void str_copy_without_ext(char *dst, const char *src, const char *ext, size_t len)
{
	const char *needle = strstr(src, ext);
	size_t sub_len = needle - src;
	strncpy(dst, src, sub_len < len ? sub_len : len);
}

void util_reload_texture(struct lodge_vfs *vfs, strview_t filename, size_t size, const void *data, void* userdata)
{
	if(size == 0) {
		reload_debug("Skipped reload of texture `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
		return;
	}

	struct lodge_image image;
	struct lodge_ret image_ret = lodge_image_new(&image, data, size);
	if(!image_ret.success) {
		reload_error("Image load failed: `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
		return;
	}

	lodge_texture_t tmp = lodge_texture_2d_make_from_image(&image);
	lodge_image_free(&image);

	if(!tmp) {
   		reload_error("Texture load failed: `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
	} else {
		if(userdata) {
			// HACK(TS): remove this
			if(strcmp("paddle.png", filename.s) == 0) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			(*(lodge_texture_t*) userdata) = tmp;
		} else {
			reload_debug("Unassigned texture: `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
			lodge_texture_reset(tmp);
		}
	}
}

void util_reload_texture_pixels(struct lodge_vfs *vfs, strview_t filename, size_t size, const void *data, void* userdata, int width, int height)
{
	if(size == 0) {
		reload_debug("Skipped reload of texture `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
		return;
	}

	struct lodge_image image;
	struct lodge_ret image_ret = lodge_image_new(&image, data, size);
	if(!image_ret.success) {
		reload_error("Image load failed: `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
		return;
	}

	lodge_texture_t tmp = lodge_texture_2d_make_from_image(&image);
	lodge_image_free(&image);

	if(!tmp) {
		reload_error("Texture load failed: `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
	} else {
		if(userdata) {
			(*(lodge_texture_t *) userdata) = tmp;
		} else {
			reload_debug("Unassigned texture: `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
			lodge_texture_reset(tmp);
		}
	}
}

#ifdef ENABLE_LODGE_CONSOLE
void util_reload_console_conf(struct lodge_vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata)
{
	if(size == 0) {
		console_debug("Skipped reload of `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
		return;
	}

	struct console *dst = (struct console *) userdata;
	dst->conf.data = (const char *) data;
	dst->conf.len = size;

	/* Is this a reload? Then re-parse config now. Otherwise the config will
	 * be parsed after initializing the console for the first time. */
	if(dst->initialized) {
		console_parse_conf(dst, &dst->conf);
	}
}
#endif

#ifdef ENABLE_LODGE_ASSET_ATLAS
void util_reload_atlas(struct lodge_vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata)
{
	if(size == 0) {
		atlas_debug("Skipped reload of `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
		return;
	}

	struct atlas *dst = (struct atlas *) userdata;

	if(!dst) {
		atlas_error("Invalid argument to util_reload_atlas()\n");
		return;
	}

	struct atlas tmp = { 0 };

	int ret = atlas_load(&tmp, data);
	if(ret != ATLAS_OK) {
		atlas_error("Error %d when loading atlas `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", ret, STRVIEW_PRINTF_ARG(filename), size);
	} else {
		/* Delete the old atlas. */
		atlas_free(dst);
		/* Assign the new atlas only if parsing succeeded. */
		(*dst) = tmp;
		/* DEBUG: Dump debug information about atlas to stdout. */
		atlas_print(dst);
	}
}
#endif

#ifdef ENABLE_LODGE_ASSET_PYXEL
void util_reload_pyxel_asset(struct lodge_vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata)
{
	if(size == 0) {
		pyxel_debug("Skipped reload of %s (%zu bytes)\n", filename, size);
		return;
	}

	struct pyxel_asset *dst = (struct pyxel_asset *) userdata;

	if(!dst) {
		pyxel_error("Invalid argument to util_reload_pyxel()\n");
		return;
	}

	struct pyxel_asset tmp = { 0 };

	int ret = pyxel_asset_load(&tmp, data, size);
	if(ret != PYXEL_OK) {
		pyxel_error("Error %d when loading pyxel %s (%zu bytes)\n", ret, filename, size);
	} else {
		pyxel_asset_free(dst);
		(*dst) = tmp;
	}
}
#endif
