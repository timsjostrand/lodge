/**
 * Loader helpers.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include "util_reload.h"

#include "vfs.h"
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

void util_reload_sound(struct vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata)
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

#if 0
static void util_reload_shader_register_callbacks(struct vfs *vfs, struct shader *s)
{
	char tmp[SHADER_FILENAME_MAX] = { 0 };
	strbuf_t buf = strbuf_wrap(tmp);

	strbuf_set(buf, strview_make(s->name, strnlen(s->name, SHADER_FILENAME_MAX)));
	strbuf_append(buf, strview_static(".frag"));
	vfs_register_callback_filter(vfs, strbuf_to_strview(buf), &util_reload_shader, s);

	strbuf_set(buf, strview_make(s->name, strnlen(s->name, SHADER_FILENAME_MAX)));
	strbuf_append(buf, strview_static(".vert"));
	vfs_register_callback_filter(vfs, strbuf_to_strview(buf), &util_reload_shader, s);

	array_foreach(s->vert_includes, const char, include) {
		vfs_register_callback(vfs, strview_make(include, strlen(include)), &util_reload_shader, s);
	}

	array_foreach(s->frag_includes, const char, include) {
		vfs_register_callback(vfs, strview_make(include, strlen(include)), &util_reload_shader, s);
	}
}

void util_reload_shader(struct vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata)
{
	if(size == 0) {
		shader_debug("Skipped reload of `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
		return;
	}

	struct shader *dst = (struct shader *) userdata;

	if(!dst) {
		shader_error("Invalid argument to util_reload_shader()\n");
		return;
	}

	/* Keep references to shader sources. */
	// FIXME(TS): safe `strstr`
	if(strstr(filename.s, ".frag") != NULL) {
		str_copy_without_ext(dst->name, filename.s, ".frag", SHADER_FILENAME_MAX);
		dst->frag_src = strview_make(data, size);
	} else if(strstr(filename.s, ".vert") != NULL) {
		str_copy_without_ext(dst->name, filename.s, ".vert", SHADER_FILENAME_MAX);
		dst->vert_src = strview_make(data, size);
	} else {
		shader_debug("%s: Included file \"" STRVIEW_PRINTF_FMT "\" modified, reloading\n", dst->name, STRVIEW_PRINTF_ARG(filename));
	}

	/* Have both sources been loaded? */
	if(dst->vert_src.length == 0) {
		shader_debug("%s: Awaiting vertex shader source...\n", dst->name);
		return;
	}
	if(dst->frag_src.length == 0) {
		shader_debug("%s: Awaiting fragment shader source...\n", dst->name);
		return;
	}

	/* Recompile shader. */
	struct shader tmp = (*dst);

	int ret = shader_init(&tmp, dst->name, dst->vert_src, dst->frag_src, vfs);
	if(ret != SHADER_OK) {
		shader_error("Error %d when loading shader `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", ret, STRVIEW_PRINTF_ARG(filename), size);
	} else {
		/* Clear callbacks for this shader (due to more being added for included files) */
		vfs_prune_callbacks(vfs, &util_reload_shader, userdata);
		/* Delete the old shader. */
		shader_delete(dst);
		//shader_free(dst);
		/* Assign the new shader only if compilation succeeded. */
		(*dst) = tmp;
		/* Relocate uniforms in the shader, if they changed. */
		shader_uniforms_relocate(dst);
		/* Trigger reloads when included files change */
		util_reload_shader_register_callbacks(vfs, dst);
	}
}
#endif

void util_reload_texture(struct vfs *vfs, strview_t filename, size_t size, const void *data, void* userdata)
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

	lodge_texture_t tmp = lodge_texture_make_from_image(&image);
	lodge_image_free(&image);

	if(!lodge_texture_is_valid(tmp)) {
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
			lodge_texture_reset(&tmp);
		}
	}
}

void util_reload_texture_pixels(struct vfs *vfs, strview_t filename, size_t size, const void *data, void* userdata, int width, int height)
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

	lodge_texture_t tmp = lodge_texture_make_from_image(&image);
	lodge_image_free(&image);

	if(!lodge_texture_is_valid(tmp)) {
		reload_error("Texture load failed: `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
	} else {
		if(userdata) {
			(*(lodge_texture_t *) userdata) = tmp;
		} else {
			reload_debug("Unassigned texture: `" STRVIEW_PRINTF_FMT "` (%zu bytes)\n", STRVIEW_PRINTF_ARG(filename), size);
			lodge_texture_reset(&tmp);
		}
	}
}

#ifdef ENABLE_LODGE_CONSOLE
void util_reload_console_conf(struct vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata)
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
void util_reload_atlas(struct vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata)
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
void util_reload_pyxel_asset(struct vfs *vfs, strview_t filename, size_t size, const void *data, void *userdata)
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
