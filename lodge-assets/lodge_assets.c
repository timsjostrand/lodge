#include "lodge_assets.h"

#include "array.h"
#include "membuf.h"
#include "str.h"
#include "lodge_hash.h"
#include "lodge_assert.h"
#include "lodge_platform.h"
#include "log.h"

#include <stdlib.h>

#define LODGE_ASSETS_MAX				256
#define LODGE_ASSET_NAME_MAX			128
#define LODGE_ASSET_DEPS_MAX			32
#define LODGE_ASSET_USERDATA_MAX		16
#define LODGE_ASSET_DATA_COUNT_MAX		1024

struct lodge_assets;

struct lodge_asset_deps
{
	struct lodge_asset_handle		deps[LODGE_ASSET_DEPS_MAX];
	size_t							count;
};

struct lodge_assets
{
	struct lodge_assets_desc		desc;

	char							names[LODGE_ASSETS_MAX][LODGE_ASSET_NAME_MAX];
	lodge_asset_id_t				ids[LODGE_ASSETS_MAX];
	uint32_t						ref_counts[LODGE_ASSETS_MAX];
	uint32_t						count;

	array_t							data;
	uint32_t						data_indices[LODGE_ASSETS_MAX];

	struct lodge_asset_deps			deps[LODGE_ASSETS_MAX];

	void*							userdata[LODGE_ASSET_USERDATA_MAX]; 

	// TODO(TS): reverse deps also?
};

// TODO(TS): sort by asset_name, binary search get

static int64_t lodge_assets_find_by_name(const struct lodge_assets *asset, strview_t name)
{
	for(uint32_t i = 0, count = asset->count; i < count; i++) {
		strview_t asset_name = strview_wrap(asset->names[i]);

		if(strview_equals(asset_name, name)) {
			ASSERT(i < LODGE_ASSETS_MAX);
			return i;
		}
	}
	return -1;
}

//
// TODO(TS): Sorted + binary search
//
static int64_t lodge_assets_find_by_id(const struct lodge_assets *assets, lodge_asset_id_t id)
{
	for(uint32_t i = 0, count = assets->count; i < count; i++) {
		if(assets->ids[i] == id) {
			ASSERT(i < LODGE_ASSETS_MAX);
			return i;
		}
	}
	return -1;
}

struct lodge_assets* lodge_assets_new(struct lodge_assets_desc desc)
{
	struct lodge_assets* assets = (struct lodge_assets * )malloc(sizeof(struct lodge_assets));
	lodge_assets_new_inplace(assets, desc);
	return assets;
}

void lodge_assets_new_inplace(struct lodge_assets *assets, struct lodge_assets_desc desc)
{
	memset(assets, 0, sizeof(struct lodge_assets));

	assets->desc = desc;
	assets->data = array_new(desc.size, LODGE_ASSET_DATA_COUNT_MAX);

	memset(assets->data->data, 0, assets->data->max_count * assets->data->element_size);
}

void lodge_assets_free_inplace(struct lodge_assets *assets)
{
	array_free(assets->data);
}

void lodge_assets_free(struct lodge_assets *assets)
{
	lodge_assets_free_inplace(assets);
	free(assets);
}

size_t lodge_assets_sizeof()
{
	return sizeof(struct lodge_assets);
}

static const void* lodge_assets_get_or_load_index(struct lodge_assets *assets, strview_t name, int64_t index)
{
#if 0
	debugf("Assets", "Get: `" STRVIEW_PRINTF_FMT "`:`" STRVIEW_PRINTF_FMT "`\n",
		STRVIEW_PRINTF_ARG(assets->desc.name),
		STRVIEW_PRINTF_ARG(name)
	);
#endif

	//
	// TODO(TS): should use `data_index` to indicate whether data has been unloaded or not.
	//

	if(index >= 0) {
		uint32_t *refcount = &assets->ref_counts[index];
		//ASSERT(*refcount > 0);
		(*refcount)++;
		const uint32_t data_index = assets->data_indices[index];
		return array_get(assets->data, (size_t)data_index);
	} else {
		index = assets->count;

		uint32_t *refcount = &assets->ref_counts[index];
		ASSERT(*refcount == 0);

		uint32_t name_hash = lodge_hash_murmur3_32(name.s, name.length);
		lodge_asset_id_t asset_id = lodge_asset_id_make(name_hash);

		// FIXME(TS): leaking empty slots when `new_inplace` fails
		void* data_element = array_append_no_init(assets->data);

		if(!assets->desc.new_inplace(assets, name, asset_id, data_element, assets->desc.size)) {
			if(data_element) {
				assets->desc.free_inplace(assets, name, asset_id, data_element);
			}
			return NULL;
		}

		assets->count = index + 1;

		const size_t data_index = array_count(assets->data) - 1;

		assets->data_indices[index] = (uint32_t)data_index;

		strbuf_wrap_and(assets->names[index], strbuf_set, name);

		assets->ids[index] = asset_id;

		ASSERT(lodge_asset_id_is_valid(assets->ids[index]));
		ASSERT(name_hash == lodge_asset_id_get_hash(assets->ids[index]));

		(*refcount)++;

		return data_element;
	}
}

const void* lodge_assets_get(struct lodge_assets *assets, strview_t name)
{
	const int64_t index = lodge_assets_find_by_name(assets, name);
	return lodge_assets_get_or_load_index(assets, name, index);
}

const void* lodge_assets_get_by_id(struct lodge_assets *assets, lodge_asset_id_t id)
{
	const int64_t index = lodge_assets_find_by_id(assets, id);
	return lodge_assets_get_or_load_index(assets, strview_wrap(assets->names[index]), index);
}

static void lodge_assets_release_index(struct lodge_assets *assets, size_t index)
{
	debugf("Assets", "Release: `" STRVIEW_PRINTF_FMT "`:`" STRVIEW_PRINTF_FMT "`\n",
		STRVIEW_PRINTF_ARG(assets->desc.name),
		STRVIEW_PRINTF_ARG(strview_wrap(assets->names[index]))
	);

	uint32_t *refcount = &assets->ref_counts[index];
	ASSERT(*refcount > 0);

	(*refcount)--;

	if(*refcount == 0) {
#if 0
		ASSERT_FAIL("Refcount == 0");

		const uint32_t data_index = res->data_indices[index];
		void *res_data = array_get(res->data, data_index);
		res->desc.free_inplace(res, name, res_data);
#endif
	}

	// TODO(TS): vacuum dependencies. maybe release dependencies also?
}

void lodge_assets_release(struct lodge_assets *assets, strview_t name)
{
	const int64_t index = lodge_assets_find_by_name(assets, name);
	if(index < 0) {
		ASSERT_FAIL("Asset not found");
		return;
	}
	lodge_assets_release_index(assets, (size_t)index);
}

static void lodge_assets_reload_by_index(struct lodge_assets *assets, size_t index);

static void lodge_assets_reload_by_id(struct lodge_assets *assets, lodge_asset_id_t id)
{
	ASSERT(lodge_asset_id_is_valid(id));

	const int64_t index = lodge_assets_find_by_id(assets, id);
	if(index < 0) {
		ASSERT_FAIL("Asset not found");
		return;
	}
	lodge_assets_reload_by_index(assets, index);
}

static void lodge_assets_reload_by_index(struct lodge_assets *assets, size_t index)
{
	ASSERT(index < LODGE_ASSETS_MAX);

	// FIXME(TS): should try reload first, and roll back changes on failure

	if(assets->desc.reload_inplace)
	{
		ASSERT_NOT_IMPLEMENTED();
	}
	else
	{
		const size_t data_index = assets->data_indices[index];
		void* data_element = array_get(assets->data, data_index);
		strview_t name = strbuf_to_strview(strbuf_wrap(assets->names[index]));
		lodge_asset_id_t id = assets->ids[index];

		assets->desc.free_inplace(assets, name, id, data_element);

		if(!assets->desc.new_inplace(assets, name, id, data_element, assets->desc.size)) {
			ASSERT_FAIL("TODO: implement rollback");
			return;
		}
	}

	for(size_t dep_index = 0, count = assets->deps[index].count; dep_index < count; dep_index++) {
		struct lodge_asset_handle *asset_handle = &assets->deps[index].deps[dep_index];
		lodge_assets_reload_by_id(asset_handle->assets, asset_handle->id);
	}
}

void lodge_assets_reload(struct lodge_assets *assets, strview_t name)
{
	debugf("Assets", "Reload: `" STRVIEW_PRINTF_FMT "`:`" STRVIEW_PRINTF_FMT "`\n",
		STRVIEW_PRINTF_ARG(assets->desc.name),
		STRVIEW_PRINTF_ARG(name)
	);

	const int64_t index = lodge_assets_find_by_name(assets, name);
	if(index < 0) {
		ASSERT_FAIL("Asset not found");
		return;
	}

	lodge_assets_reload_by_index(assets, index);
}

static bool lodge_asset_handle_equals(struct lodge_asset_handle *a, struct lodge_asset_handle *b)
{
	return (a->assets == b->assets) && (a->id == b->id);
}

static int lodge_assets_deps_find_handle(struct lodge_asset_deps *deps, struct lodge_asset_handle dependency)
{
	for(size_t i = 0; i < deps->count; i++) {
		if(lodge_asset_handle_equals(&deps->deps[i], &dependency)) {
			return i;
		}
	}
	return -1;
}

static void lodge_assets_append_dep(struct lodge_assets *assets, size_t index, struct lodge_asset_handle dependency)
{
	ASSERT(index < LODGE_ASSETS_MAX);
	struct lodge_asset_deps *deps = &assets->deps[index];

	//
	// No duplicate dependencies
	//
	ASSERT(lodge_assets_deps_find_handle(deps, dependency) == -1);

	//
	// FIXME(TS): need to release dependencies when freeing a `asset` or leak dep list
	//
	ASSERT(deps->count + 1 < LODGE_ASSET_DEPS_MAX);
	if(deps->count + 1 < LODGE_ASSET_DEPS_MAX) {
		const size_t dep_index = assets->deps[index].count++;
		ASSERT(dep_index < LODGE_ASSET_DEPS_MAX);
		assets->deps[index].deps[dep_index] = dependency;

#if 0
		strview_t dep_name = lodge_asset_handle_find_name(&deps->deps[dep_index]);
		debugf("Assets", "New dependency: `" STRVIEW_PRINTF_FMT "`:`" STRVIEW_PRINTF_FMT "` from `" STRVIEW_PRINTF_FMT "`:`%s` (count: %zd)`\n",
			STRVIEW_PRINTF_ARG(deps->deps[dep_index].assets->desc.name),
			STRVIEW_PRINTF_ARG(dep_name),
			STRVIEW_PRINTF_ARG(assets->desc.name),
			assets->names[index],
			deps->count
		);
#endif
	}
}

static void lodge_assets_remove_dep(struct lodge_assets *assets, size_t index, struct lodge_asset_handle dependency)
{
	ASSERT(index < LODGE_ASSETS_MAX);
	struct lodge_asset_deps *deps = &assets->deps[index];

	for(size_t dep_index = 0, count = deps->count; dep_index < count; dep_index++) {
		struct lodge_asset_handle *handle = &deps->deps[dep_index];
		if(lodge_asset_handle_equals(handle, &dependency)) {
#if 0
			strview_t dep_name = lodge_asset_handle_find_name(&deps->deps[dep_index]);
			debugf("Assets", "Lost dependency: `" STRVIEW_PRINTF_FMT "` from `%s` (count: %zd)`\n",
				STRVIEW_PRINTF_ARG(dep_name),
				assets->names[index],
				deps->count
			);
#endif

			membuf_t deps_membuf = membuf_wrap(deps->deps);
			membuf_delete_swap_tail(deps_membuf, &deps->count, dep_index);

			return;
		}
	}

	ASSERT_FAIL("Dependency not found");
}

const void* lodge_assets_get_depend(struct lodge_assets *assets, strview_t name, struct lodge_asset_handle dependency)
{
	// TODO(TS): need to check if this asset is already a dependency (only once to not trigger reload multiple times)
	const void* data = lodge_assets_get(assets, name);
	if(!data) {
		return NULL;
	}

	const int64_t index = lodge_assets_find_by_name(assets, name);
	if(index < 0) {
		ASSERT_FAIL("Asset not found");
		return NULL;
	}

	lodge_assets_append_dep(assets, (size_t)index, dependency);

	return data;
}

void lodge_assets_release_depend(struct lodge_assets *assets, strview_t name, struct lodge_asset_handle dependency)
{
	const int64_t index = lodge_assets_find_by_name(assets, name);
	if(index < 0) {
		ASSERT_FAIL("Asset not found");
		return;
	}

	lodge_assets_release_index(assets, (size_t)index);
	lodge_assets_remove_dep(assets, (size_t)index, dependency);
}

//
// Removes all this dependency from all assets in the asset manager that may be depending on it.
//
void lodge_assets_clear_dependency(struct lodge_assets *assets, struct lodge_asset_handle dependency)
{
	//
	// TODO(TS): should have a reverse dependency lookup for this instead
	//

	for(uint32_t asset_index = 0; asset_index < assets->count; asset_index++) {
		struct lodge_asset_deps *deps = &assets->deps[asset_index];
		
		for(size_t dep_index = 0; dep_index < deps->count; dep_index++) {
			if(lodge_asset_handle_equals(&deps->deps[dep_index], &dependency)) {
				lodge_assets_release_index(assets, (size_t)asset_index);
				lodge_assets_remove_dep(assets, (size_t)asset_index, dependency);

				//
				// NOTE(TS): `lodge_assets_remove_dep()` will delete_swap_tail
				//
				dep_index--;
			}
		}
	}
}

void lodge_assets_set_userdata(struct lodge_assets *assets, size_t index, void *userdata)
{
	ASSERT(index < LODGE_ASSET_USERDATA_MAX);
	assets->userdata[index] = userdata;
}

void* lodge_assets_get_userdata(struct lodge_assets *assets, size_t index)
{
	ASSERT(index < LODGE_ASSET_USERDATA_MAX);
	return assets->userdata[index];
}

strview_t lodge_assets_id_to_name(struct lodge_assets *assets, lodge_asset_id_t id)
{
	ASSERT(lodge_asset_id_is_valid(id));

	for(size_t i = 0, count = assets->count; i < count; i++) {
		if(assets->ids[i] == id) {
			return strbuf_to_strview(strbuf_wrap(assets->names[i]));
		}
	}
	
	ASSERT_FAIL("Asset ID not found");
	return strview_static("n/a");
}

uint32_t lodge_assets_get_count(struct lodge_assets *assets)
{
	return assets->count;
}

strview_t lodge_assets_index_to_name(struct lodge_assets *assets, uint32_t index)
{
	return strview_wrap(assets->names[index]);
}

lodge_asset_id_t lodge_assets_name_to_id(struct lodge_assets *assets, strview_t name)
{
	for(size_t i = 0, count = assets->count; i < count; i++) {
		if(strview_equals(strview_wrap(assets->names[i]), name)) {
			return assets->ids[i];
		}
	}
	
	ASSERT_FAIL("Asset name not found");
	return lodge_asset_id_make_invalid();
}

strview_t lodge_asset_handle_find_name(struct lodge_asset_handle *handle)
{
	return lodge_assets_id_to_name(handle->assets, handle->id);
}