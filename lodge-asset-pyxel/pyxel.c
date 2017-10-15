/**
 * API for parsing .pyxel files.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <libarchive/archive.h>
#include <libarchive/archive_entry.h>

#include "pyxel.h"
#include "str.h"

static int pyxel_parse_str(char *dst, cJSON *item, const char *key);
static int pyxel_parse_int(int *dst, cJSON *item, const char *key);
static int pyxel_parse_int_from(int *dst, cJSON *value);

static int pyxel_parse_str(char *dst, cJSON *item, const char *key)
{
	if(item == NULL) {
		pyxel_error("item == NULL\n");
		return PYXEL_ERROR;
	}
	cJSON *value = cJSON_GetObjectItem(item, key);
	if(value == NULL) {
		pyxel_error("Invalid key \"%s\"\n", key);
		return PYXEL_ERROR;
	}
	if(value->type != cJSON_String) {
		pyxel_error("Value \"%s\" type != string (%d)\n", key, value->type);
		return PYXEL_ERROR;
	}
	strncpy(dst, value->valuestring, PYXEL_STR_MAX);
	return PYXEL_OK;
}

static int pyxel_parse_int_from(int *dst, cJSON *value)
{
	if(value == NULL) {
		pyxel_error("value == NULL\n");
		return PYXEL_ERROR;
	}
	if(value->type != cJSON_Number
			&& value->type != cJSON_False
			&& value->type != cJSON_True) {
		pyxel_error("Value \"%s\" type != number (%d)\n", value->string, value->type);
		return PYXEL_ERROR;
	}
	*dst = value->valueint;
	return PYXEL_OK;
}

static int pyxel_parse_int(int *dst, cJSON *item, const char *key)
{
	if(item == NULL) {
		pyxel_error("item == NULL\n");
		return PYXEL_ERROR;
	}
	cJSON *value = cJSON_GetObjectItem(item, key);
	if(value == NULL) {
		pyxel_error("Invalid key \"%s\"\n", key);
		return PYXEL_ERROR;
	}
	return pyxel_parse_int_from(dst, value);
}

static int pyxel_archive_open(void *data, size_t data_len, struct archive **archive)
{
	struct archive *tmp = archive_read_new();

	/* Enable all compressions and formats. */
	if(archive_read_support_filter_all(tmp) != ARCHIVE_OK
			|| archive_read_support_format_all(tmp) != ARCHIVE_OK) {
		pyxel_error("Format error: %d %s\n", archive_errno(tmp), archive_error_string(tmp));
		goto error;
	}

	/* Read archive from memory. */
	if(archive_read_open_memory(tmp, data, data_len) != ARCHIVE_OK) {
		pyxel_error("Open error: %d %s\n", archive_errno(tmp), archive_error_string(tmp));
		goto error;
	}

	*archive = tmp;

	return PYXEL_OK;

error:
	archive_read_free(tmp);
	return PYXEL_ERROR;
}

static int pyxel_archive_find(struct archive *archive, const char *filename, struct archive_entry **entry)
{
	/* Iterate archive entries. */
	struct archive_entry *tmp;
	while(archive_read_next_header(archive, &tmp) == ARCHIVE_OK) {
		if(strcmp(filename, archive_entry_pathname(tmp)) != 0) {
			continue;
		} else {
			*entry = tmp;
			return PYXEL_OK;
		}
	}

	return PYXEL_ERROR;
}

int pyxel_archive_extract(void *data, size_t data_len, char *filename, char *dst, size_t dst_len)
{
	struct archive *a = 0;
	if(pyxel_archive_open(data, data_len, &a) != PYXEL_OK) {
		goto error;
	}

	struct archive_entry *entry = 0;
	if(pyxel_archive_find(a, filename, &entry) != PYXEL_OK) {
		goto error;
	}

	/* Extract entry. */
	size_t entry_size = archive_entry_size(entry);

	if(entry_size > 0) {
		int r = archive_read_data(a, dst, dst_len);
		if(r < ARCHIVE_OK) {
			pyxel_error("Read failed: %d %s\n", archive_errno(a), archive_error_string(a));
			goto error;
		}
		if(r < ARCHIVE_WARN) {
			goto error;
		}
	} else {
		pyxel_debug("\"%s\": size <= 0\n", filename);
	}

	/* Cleanup. */
	archive_read_free(a);
	return PYXEL_OK;

error:
	archive_read_free(a);
	return PYXEL_ERROR;
}

static void pyxel_frame_duration_multipliers_check(cJSON *anim, const char (*name)[PYXEL_ANIM_NAME_MAX])
{
	cJSON *frame_duration_multipliers = cJSON_GetObjectItem(anim, "frameDurationMultipliers");
	int count = cJSON_GetArraySize(frame_duration_multipliers);
	int last = 0;

	for(int i=0; i<count; i++) {
		cJSON *elem = cJSON_GetArrayItem(frame_duration_multipliers, i);
		int duration;
		pyxel_parse_int_from(&duration, elem);
		if(i != 0 && last != duration) {
			pyxel_error("\"%s\": Different per-frame times are not supported!\n", name);
			return;
		}
		last = duration;
	}
}

static int pyxel_parse_doc_data(struct pyxel *pyxel, const char *data)
{
	cJSON *root = cJSON_Parse(data);

	/* "canvas" branch. */
	cJSON *canvas = cJSON_GetObjectItem(root, "canvas");
	PYXEL_OK_OR_BAIL(pyxel_parse_int(&pyxel->tile_width, canvas, "tileWidth"));
	PYXEL_OK_OR_BAIL(pyxel_parse_int(&pyxel->tile_height, canvas, "tileHeight"));
	PYXEL_OK_OR_BAIL(pyxel_parse_int(&pyxel->num_layers, canvas, "numLayers"));
	PYXEL_OK_OR_BAIL(pyxel_parse_int(&pyxel->width, canvas, "width"));
	PYXEL_OK_OR_BAIL(pyxel_parse_int(&pyxel->height, canvas, "height"));

	/* "canvas.layers" branch. */
	cJSON *layers = cJSON_GetObjectItem(canvas, "layers");
	pyxel->layers_count = cJSON_GetArraySize(layers);

	/* Sanity check. */
	if(pyxel->layers_count > PYXEL_LAYERS_MAX) {
		pyxel_error("Too many layers (%d, max supported: %d)!\n", pyxel->layers_count, PYXEL_LAYERS_MAX);
		goto bail;
	}

	/* Iterate "canvas.layers" branch. */
	for(int i=0; i<pyxel->layers_count; i++) {
		/* Parse iterated layer. */
		cJSON *layer = cJSON_GetArrayItem(layers, i);
		PYXEL_OK_OR_BAIL(pyxel_parse_str(pyxel->layers[i].name, layer, "name"));
	}

	/* "animations" branch. */
	cJSON *anims = cJSON_GetObjectItem(root, "animations");
	pyxel->anims_count = cJSON_GetArraySize(anims);

	/* Sanity check. */
	if(pyxel->anims_count > PYXEL_ANIMS_MAX) {
		pyxel_error("Too many animations (%d, max supported: %d)!\n", pyxel->anims_count, PYXEL_ANIMS_MAX);
		goto bail;
	}

	/* Iterate "animations" branch. */
	for(int i=0; i<pyxel->anims_count; i++) {
		/* Parse iterated animation. */
		cJSON *anim_leaf = cJSON_GetArrayItem(anims, i);
		PYXEL_OK_OR_BAIL(pyxel_parse_str(pyxel->anims[i].name, anim_leaf, "name"));

		/* Sanity check for "frameDurationMultipliers". */
		pyxel_frame_duration_multipliers_check(anim_leaf, &pyxel->anims[i].name);

		PYXEL_OK_OR_BAIL(pyxel_parse_int(&pyxel->anims[i].base_tile, anim_leaf, "baseTile"));
		PYXEL_OK_OR_BAIL(pyxel_parse_int(&pyxel->anims[i].length, anim_leaf, "length"));
		PYXEL_OK_OR_BAIL(pyxel_parse_int(&pyxel->anims[i].frame_duration, anim_leaf, "frameDuration"));
	}

	/* Normal cleanup. */
	cJSON_Delete(root);
	return PYXEL_OK;

bail:
	cJSON_Delete(root);
	return PYXEL_ERROR;
}

int pyxel_load(struct pyxel *pyxel, void *data, size_t data_len)
{
	/* Extract "docData.json". */
	char doc_data[PYXEL_DOC_DATA_MAX] = { 0 };
	if(pyxel_archive_extract(data, data_len,
				"docData.json",
				doc_data, sizeof(doc_data)) != PYXEL_OK) {
		pyxel_error("Could not extract docData.json\n");
		return PYXEL_ERROR;
	}

	/* Parse "docData.json". */
	if(pyxel_parse_doc_data(pyxel, doc_data) != PYXEL_OK) {
		pyxel_error("Could not parse docData.json\n");
		return PYXEL_ERROR;
	}

	return PYXEL_OK;
}

/**
 * Retrieve the size of a file in the .pyxel archive.
 */
int pyxel_archive_file_size(void *data, const size_t data_len, const char *filename, size_t *size)
{
	struct archive *a;
	if(pyxel_archive_open(data, data_len, &a) != PYXEL_OK) {
		goto error;
	}

	struct archive_entry *entry;
	if(pyxel_archive_find(a, filename, &entry) != PYXEL_OK) {
		goto error;
	}

	/* Extract entry. */
	*size = (size_t)archive_entry_size(entry);

	/* Cleanup. */
	archive_read_free(a);
	return PYXEL_OK;

error:
	*size = 0;
	archive_read_free(a);
	return PYXEL_ERROR;
}

#if 0
void pyxel_free(struct pyxel *pyxel)
{
	// Nothing to free at the moment.
}
#endif
