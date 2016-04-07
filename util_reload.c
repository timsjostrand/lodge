/**
 * Loader helpers.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <string.h>

#include "core.h"
#include "sound.h"
#include "shader.h"
#include "atlas.h"
#include "texture.h"
#include "console.h"
#include "pyxel_asset.h"

void util_reload_sound(const char *filename, unsigned int size, void *data, void *userdata)
{
	if(size == 0) {
		sound_debug("Skipped reload of %s (%u bytes)\n", filename, size);
		return;
	}

	sound_buf_t tmp = 0;
	sound_buf_t *dst = (sound_buf_t *) userdata;

	/* Reload sound. */
	if(sound_buf_load_vorbis(&tmp, data, size) != SOUND_OK) {
		sound_error("Could not load %s (%u bytes)\n", filename, size);
	} else {
		/* Release current sound (if any). */
		sound_buf_free((*dst));

		/* Assign new sound (only if parsing was OK). */
		(*dst) = tmp;
	}
}

void util_reload_shader(const char *filename, unsigned int size, void *data, void *userdata)
{
	if(size == 0) {
		shader_debug("Skipped reload of %s (%u bytes)\n", filename, size);
		return;
	}

	struct shader *dst = (struct shader *) userdata;

	if(!dst) {
		shader_error("Invalid argument to util_reload_shader()\n");
		return;
	}

	/* Keep references to shader sources. */
	if(strstr(filename, ".frag") != NULL) {
		dst->frag_src = data;
		dst->frag_src_len = size;
	} else if(strstr(filename, ".vert") != NULL) {
		dst->vert_src = data;
		dst->vert_src_len = size;
	} else {
		shader_error("Unknown source name \"%s\" (%u bytes)\n", filename, size);
		return;
	}

	/* Have both sources been loaded? */
	if(dst->vert_src_len == 0) {
		shader_debug("Awaiting vertex shader source...\n");
		return;
	}
	if(dst->frag_src_len == 0) {
		shader_debug("Awaiting fragment shader source...\n");
		return;
	}

	/* Recompile shader. */
	struct shader tmp = (*dst);

	int ret = shader_init(&tmp,
			dst->vert_src, dst->vert_src_len,
			dst->frag_src, dst->frag_src_len);
	if(ret != SHADER_OK) {
		shader_error("Error %d when loading shader %s (%u bytes)\n", ret, filename, size);
	} else {
		/* Delete the old shader. */
		shader_delete(dst);
		/* Assign the new shader only if compilation succeeded. */
		(*dst) = tmp;
		/* Relocate uniforms in the shader, if they changed. */
		shader_uniforms_relocate(dst);
	}
}

void util_reload_atlas(const char *filename, unsigned int size, void *data, void *userdata)
{
	if(size == 0) {
		atlas_debug("Skipped reload of %s (%u bytes)\n", filename, size);
		return;
	}

	struct atlas *dst = (struct atlas *) userdata;

	if(!dst) {
		atlas_error("Invalid argument to util_reload_atlas()\n");
		return;
	}

	struct atlas tmp = { 0 };

	int ret = atlas_load(&tmp, data, size);
	if(ret != ATLAS_OK) {
		atlas_error("Error %d when loading atlas %s (%u bytes)\n", ret, filename, size);
	} else {
		/* Delete the old atlas. */
		atlas_free(dst);
		/* Assign the new atlas only if parsing succeeded. */
		(*dst) = tmp;
		/* DEBUG: Dump debug information about atlas to stdout. */
		atlas_print(dst);
	}
}

void util_reload_texture(const char *filename, unsigned int size, void *data, void* userdata)
{
	if(size == 0) {
		core_debug("Skipped reload of texture %s (%u bytes)\n", filename, size);
		return;
	}

	GLuint tmp;

	int ret = texture_load(&tmp, NULL, NULL, data, size);

	if(ret != GRAPHICS_OK) {
		core_error("Texture load failed: %s (%u bytes)\n", filename, size);
	} else {
		if(userdata) {
			if(strcmp("paddle.png", filename) == 0) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			(*(GLuint*)userdata) = tmp;
		} else {
			core_debug("Unassigned texture: %s (%u bytes)\n", filename, size);
			texture_free(tmp);
		}
	}
}

void util_reload_texture_pixels(const char *filename, unsigned int size, void *data, void* userdata, int width, int height)
{
	if(size == 0) {
		core_debug("Skipped reload of texture %s (%u bytes)\n", filename, size);
		return;
	}

	GLuint tmp;

	int ret = texture_load_pixels(&tmp, data, width, height);

	if(ret != GRAPHICS_OK) {
		core_error("Texture load failed: %s (%u bytes)\n", filename, size);
	} else {
		if(userdata) {
			(*(GLuint *) userdata) = tmp;
		} else {
			core_debug("Unassigned texture: %s (%u bytes)\n", filename, size);
			texture_free(tmp);
		}
	}
}

void util_reload_console_conf(const char *filename, unsigned int size, void *data, void *userdata)
{
	if(size == 0) {
		console_debug("Skipped reload of %s (%u bytes)\n", filename, size);
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

void util_reload_pyxel_asset(const char *filename, unsigned int size, void *data, void *userdata)
{
	if(size == 0) {
		pyxel_debug("Skipped reload of %s (%u bytes)\n", filename, size);
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
		pyxel_error("Error %d when loading pyxel %s (%u bytes)\n", ret, filename, size);
	} else {
		pyxel_asset_free(dst);
		(*dst) = tmp;
	}
}
