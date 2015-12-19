/**
 * JSON texture atlas file parsing.
 *
 * Specifically, will parse JSON as outputted by TexturePacker when "Data
 * Format" is set to "JSON (Array)".
 *
 * The parsing is slimmed down by default, to parse _all_ the information define
 * ATLAS_FATTY.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

#include "atlas.h"
#include "str.h"

static int atlas_parse_str(char *dst, cJSON *item, const char *key);
static int atlas_parse_int(int *dst, cJSON *item, const char *key);

static int atlas_parse_str(char *dst, cJSON *item, const char *key)
{
	if(item == NULL) {
		atlas_error("item == NULL\n");
		return ATLAS_ERROR;
	}
	cJSON *value = cJSON_GetObjectItem(item, key);
	if(value == NULL) {
		atlas_error("Invalid key \"%s\"\n", key);
		return ATLAS_ERROR;
	}
	if(value->type != cJSON_String) {
		atlas_error("Value \"%s\" type != string (%d)\n", key, value->type);
		return ATLAS_ERROR;
	}
	strncpy(dst, value->valuestring, ATLAS_STR_MAX);
	return ATLAS_OK;
}

static int atlas_parse_int(int *dst, cJSON *item, const char *key)
{
	if(item == NULL) {
		atlas_error("item == NULL\n");
		return ATLAS_ERROR;
	}
	cJSON *value = cJSON_GetObjectItem(item, key);
	if(value == NULL) {
		atlas_error("Invalid key \"%s\"\n", key);
		return ATLAS_ERROR;
	}
	if(value->type != cJSON_Number
			&& value->type != cJSON_False
			&& value->type != cJSON_True) {
		atlas_error("Value \"%s\" type != number (%d)\n", key, value->type);
		return ATLAS_ERROR;
	}
	*dst = value->valueint;
	return ATLAS_OK;
}

void atlas_print(struct atlas *atlas)
{
	printf("====\n");
	printf("image:        %32s\n", atlas->image);
	printf("format:       %32s\n", atlas->format);
	printf("width:        % 32d\n", atlas->width);
	printf("height:       % 32d\n", atlas->height);
	printf("frames_count: % 32d\n", atlas->frames_count);
	printf("first frame:  % 8d% 8d% 8d% 8d\n",
			atlas->frames[0].x,
			atlas->frames[0].y,
			atlas->frames[0].width,
			atlas->frames[0].height);
	printf("last frame:   % 8d% 8d% 8d% 8d\n",
			atlas->frames[atlas->frames_count-1].x,
			atlas->frames[atlas->frames_count-1].y,
			atlas->frames[atlas->frames_count-1].width,
			atlas->frames[atlas->frames_count-1].height);
	printf("====\n");
}

#define ATLAS_TRY(e) if((ret = e) != ATLAS_OK) goto bail

int atlas_load(struct atlas *atlas, void *data, size_t data_len)
{
	int ret = 0;

	/* FIXME: isn't there a parse(data,len)? */
	cJSON *root = cJSON_Parse(data);
	cJSON *meta = cJSON_GetObjectItem(root, "meta");
	cJSON *frames = cJSON_GetObjectItem(root, "frames");

	/* Parse atlas meta information. */
	ATLAS_TRY(atlas_parse_str(atlas->image, meta, "image"));
	ATLAS_TRY(atlas_parse_str(atlas->format, meta, "format"));

	/* "meta.size" leaf. */
	cJSON *meta_size = cJSON_GetObjectItem(meta, "size");
	ATLAS_TRY(atlas_parse_int(&atlas->width, meta_size, "w"));
	ATLAS_TRY(atlas_parse_int(&atlas->height, meta_size, "h"));

	/* Count elements in "frames" array. */
	atlas->frames_count = cJSON_GetArraySize(frames);

#ifdef ATLAS_FATTY
	ATLAS_TRY(atlas_parse_str(atlas->scale, meta, "scale"));
	ATLAS_TRY(atlas_parse_str(atlas->generator, meta, "app"));
	ATLAS_TRY(atlas_parse_str(atlas->generator_version, meta, "version"));
#endif

	/* Parse frames. */
	atlas->frames = (struct atlas_frame *) calloc(atlas->frames_count, sizeof(struct atlas_frame));

	if(atlas->frames == NULL) {
		return ATLAS_ERROR;
	}

	for(int i=0; i<atlas->frames_count; i++) {
		struct atlas_frame *f = &(atlas->frames[i]);

		/* Iterate JSON array. */
		cJSON *elem = cJSON_GetArrayItem(frames, i);

		/* "frame" leaf. */
		cJSON *elem_frame = cJSON_GetObjectItem(elem, "frame");
		ATLAS_TRY(atlas_parse_int(&f->x, elem_frame, "x"));
		ATLAS_TRY(atlas_parse_int(&f->y, elem_frame, "y"));
		ATLAS_TRY(atlas_parse_int(&f->width, elem_frame, "w"));
		ATLAS_TRY(atlas_parse_int(&f->height, elem_frame, "h"));

		/* Rest of params. */
		ATLAS_TRY(atlas_parse_int(&f->rotated, elem, "rotated"));
		ATLAS_TRY(atlas_parse_int(&f->trimmed, elem, "trimmed"));

		ATLAS_TRY(atlas_parse_str(f->name, elem, "filename"));

#ifdef ATLAS_FATTY
		/* "sourceSize" leaf. */
		cJSON *elem_src = cJSON_GetObjectItem(elem, "sourceSize");
		ATLAS_TRY(atlas_parse_int(&f->src_size_w, elem_src, "w"));
		ATLAS_TRY(atlas_parse_int(&f->src_size_h, elem_src, "h"));

		/* "spriteSourceSize" leaf. */
		cJSON *elem_sprite_src = cJSON_GetObjectItem(elem, "spriteSourceSize");
		ATLAS_TRY(atlas_parse_int(&f->sprite_src_x, elem_sprite_src, "x"));
		ATLAS_TRY(atlas_parse_int(&f->sprite_src_y, elem_sprite_src, "y"));
		ATLAS_TRY(atlas_parse_int(&f->sprite_src_w, elem_sprite_src, "w"));
		ATLAS_TRY(atlas_parse_int(&f->sprite_src_h, elem_sprite_src, "h"));
#endif
	}

	/* Clean up. */
	cJSON_Delete(root);

	return ATLAS_OK;

bail:
	cJSON_Delete(root);
	atlas_free(atlas);
	return ret;
}

void atlas_free(struct atlas *atlas)
{
	if(atlas == NULL) {
		return;
	}
	free(atlas->frames);
}

/**
 * Looks for a frame name in the given atlas. If it is not found, -1 is returned.
 */
int atlas_frame_index(struct atlas *atlas, const char *name)
{
	for(int i=0; i<atlas->frames_count; i++) {
		if(str_equals(atlas->frames[i].name, name)) {
			return i;
		}
	}
	atlas_debug("Could not find frame \"%s\"\n", name);
	return -1;
}
