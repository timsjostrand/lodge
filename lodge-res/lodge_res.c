#include "lodge_res.h"

#include "array.h"
#include "membuf.h"
#include "str.h"
#include "lodge_hash.h"
#include "lodge_assert.h"
#include "lodge_platform.h"
#include "log.h"

#include <stdlib.h>

#define LODGE_RES_MAX				256
#define LODGE_RES_NAME_MAX			128
#define LODGE_RES_DEPS_MAX			32
#define LODGE_RES_USERDATA_MAX		16
#define LODGE_RES_DATA_COUNT_MAX	1024

struct lodge_res;

struct lodge_res_deps
{
	struct lodge_res_handle		deps[LODGE_RES_DEPS_MAX];
	size_t						count;
};

struct lodge_res
{
	struct lodge_res_desc		desc;

	char						names[LODGE_RES_MAX][LODGE_RES_NAME_MAX];
	lodge_res_id_t				ids[LODGE_RES_MAX];
	uint32_t					ref_counts[LODGE_RES_MAX];
	uint32_t					count;

	array_t						data;
	uint32_t					data_indices[LODGE_RES_MAX];

	struct lodge_res_deps		deps[LODGE_RES_MAX];

	void*						userdata[LODGE_RES_USERDATA_MAX]; 

	// TODO(TS): reverse deps also?
};

// TODO(TS): sort by res_name, binary search get

static int64_t lodge_res_find_by_name(const struct lodge_res *res, strview_t name)
{
	for(uint32_t i = 0, count = res->count; i < count; i++) {
		strview_t res_name = strview_wrap(res->names[i]);

		if(strview_equals(res_name, name)) {
			ASSERT(i < LODGE_RES_MAX);
			return i;
		}
	}
	return -1;
}

//
// TODO(TS): Sorted + binary search
//
static int64_t lodge_res_find_by_id(const struct lodge_res *res, lodge_res_id_t id)
{
	for(uint32_t i = 0, count = res->count; i < count; i++) {
		if(res->ids[i] == id) {
			ASSERT(i < LODGE_RES_MAX);
			return i;
		}
	}
	return -1;
}

struct lodge_res* lodge_res_new(struct lodge_res_desc desc)
{
	struct lodge_res* res = (struct lodge_res * )malloc(sizeof(struct lodge_res));
	lodge_res_new_inplace(res, desc);
	return res;
}

void lodge_res_new_inplace(struct lodge_res *res, struct lodge_res_desc desc)
{
	*res = (struct lodge_res) { 0 };

	res->desc = desc;
	res->data = array_new(desc.size, LODGE_RES_DATA_COUNT_MAX);
}

void lodge_res_free_inplace(struct lodge_res *res)
{
	array_free(res->data);
}

void lodge_res_free(struct lodge_res *res)
{
	lodge_res_free_inplace(res);
	free(res);
}

size_t lodge_res_sizeof()
{
	return sizeof(struct lodge_res);
}

static const void* lodge_res_get_or_load_index(struct lodge_res *res, strview_t name, int64_t index)
{
	debugf("Res", "Get: `" STRVIEW_PRINTF_FMT "`:`" STRVIEW_PRINTF_FMT "`\n",
		STRVIEW_PRINTF_ARG(res->desc.name),
		STRVIEW_PRINTF_ARG(name)
	);

	//
	// TODO(TS): should use `data_index` to indicate whether data has been unloaded or not.
	//

	if(index >= 0) {
		uint32_t *refcount = &res->ref_counts[index];
		//ASSERT(*refcount > 0);
		(*refcount)++;
		const uint32_t data_index = res->data_indices[index];
		return array_get(res->data, (size_t)data_index);
	} else {
		index = res->count++;

		uint32_t *refcount = &res->ref_counts[index];
		ASSERT(*refcount == 0);

		void* data_element = array_append_no_init(res->data);
		const size_t data_index = array_count(res->data) - 1;

		res->data_indices[index] = (uint32_t)data_index;

		strbuf_wrap_and(res->names[index], strbuf_set, name);

		uint32_t hash = lodge_hash_murmur3_32(name.s, name.length);
		res->ids[index] = lodge_res_id_make(hash);

		ASSERT(lodge_res_id_is_valid(res->ids[index]));
		ASSERT(hash == lodge_res_id_get_hash(res->ids[index]));

		// TODO(TS): check return of new_inplace

		if(!res->desc.new_inplace(res, name, res->ids[index], data_element, res->desc.size)) {
			// FIXME(TS): roll back and clean up after failed new_inplace
			ASSERT_NOT_IMPLEMENTED();
			return NULL;
		}

		(*refcount)++;

		return data_element;
	}
}

const void* lodge_res_get(struct lodge_res *res, strview_t name)
{
	const int64_t index = lodge_res_find_by_name(res, name);
	return lodge_res_get_or_load_index(res, name, index);
}

static void lodge_res_release_index(struct lodge_res *res, size_t index)
{
	debugf("Res", "Release: `" STRVIEW_PRINTF_FMT "`:`" STRVIEW_PRINTF_FMT "`\n",
		STRVIEW_PRINTF_ARG(res->desc.name),
		STRVIEW_PRINTF_ARG(strview_wrap(res->names[index]))
	);

	uint32_t *refcount = &res->ref_counts[index];
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

void lodge_res_release(struct lodge_res *res, strview_t name)
{
	const int64_t index = lodge_res_find_by_name(res, name);
	if(index < 0) {
		ASSERT_FAIL("Asset not found");
		return;
	}
	lodge_res_release_index(res, (size_t)index);
}

static void lodge_res_reload_by_index(struct lodge_res *res, size_t index);

static void lodge_res_reload_by_id(struct lodge_res *res, lodge_res_id_t id)
{
	ASSERT(lodge_res_id_is_valid(id));

	const int64_t index = lodge_res_find_by_id(res, id);
	if(index < 0) {
		ASSERT_FAIL("Asset not found");
		return;
	}
	lodge_res_reload_by_index(res, index);
}

static void lodge_res_reload_by_index(struct lodge_res *res, size_t index)
{
	ASSERT(index < LODGE_RES_MAX);

	// FIXME(TS): should try reload first, and roll back changes on failure

	if(res->desc.reload_inplace)
	{
		ASSERT_NOT_IMPLEMENTED();
	}
	else
	{
		const size_t data_index = res->data_indices[index];
		void* data_element = array_get(res->data, data_index);
		strview_t name = strbuf_to_strview(strbuf_wrap(res->names[index]));
		lodge_res_id_t id = res->ids[index];

		res->desc.free_inplace(res, name, id, data_element);

		if(!res->desc.new_inplace(res, name, id, data_element, res->desc.size)) {
			ASSERT_FAIL("TODO: implement rollback");
			return;
		}
	}

	for(size_t dep_index = 0, count = res->deps[index].count; dep_index < count; dep_index++) {
		struct lodge_res_handle *res_handle = &res->deps[index].deps[dep_index];
		lodge_res_reload_by_id(res_handle->resources, res_handle->id);
	}
}

void lodge_res_reload(struct lodge_res *res, strview_t name)
{
	debugf("Res", "Reload: `" STRVIEW_PRINTF_FMT "`:`" STRVIEW_PRINTF_FMT "`\n",
		STRVIEW_PRINTF_ARG(res->desc.name),
		STRVIEW_PRINTF_ARG(name)
	);

	const int64_t index = lodge_res_find_by_name(res, name);
	if(index < 0) {
		ASSERT_FAIL("Asset not found");
		return;
	}

	lodge_res_reload_by_index(res, index);
}

static bool lodge_res_handle_equals(struct lodge_res_handle *a, struct lodge_res_handle *b)
{
	return (a->resources == b->resources) && (a->id == b->id);
}

static void lodge_res_append_dep(struct lodge_res *res, size_t index, struct lodge_res_handle dependency)
{
	ASSERT(index < LODGE_RES_MAX);
	struct lodge_res_deps *deps = &res->deps[index];

	//
	// FIXME(TS): need to release dependencies when freeing a `res` or leak dep list
	//
	ASSERT(deps->count + 1 < LODGE_RES_DEPS_MAX);
	if(deps->count + 1 < LODGE_RES_DEPS_MAX) {
		const size_t dep_index = res->deps[index].count++;
		ASSERT(dep_index < LODGE_RES_DEPS_MAX);
		res->deps[index].deps[dep_index] = dependency;

		strview_t dep_name = lodge_res_handle_find_name(&deps->deps[dep_index]);

		debugf("Res", "New dependency: `" STRVIEW_PRINTF_FMT "`:`" STRVIEW_PRINTF_FMT "` from `" STRVIEW_PRINTF_FMT "`:`%s` (count: %zd)`\n",
			STRVIEW_PRINTF_ARG(deps->deps[dep_index].resources->desc.name),
			STRVIEW_PRINTF_ARG(dep_name),
			STRVIEW_PRINTF_ARG(res->desc.name),
			res->names[index],
			deps->count
		);
	}
}

static void lodge_res_remove_dep(struct lodge_res *res, size_t index, struct lodge_res_handle dependency)
{
	ASSERT(index < LODGE_RES_MAX);
	struct lodge_res_deps *deps = &res->deps[index];

	for(size_t dep_index = 0, count = deps->count; dep_index < count; dep_index++) {
		struct lodge_res_handle *handle = &deps->deps[dep_index];
		if(lodge_res_handle_equals(handle, &dependency)) {
			strview_t dep_name = lodge_res_handle_find_name(&deps->deps[dep_index]);
			debugf("Res", "Lost dependency: `" STRVIEW_PRINTF_FMT "` from `%s` (count: %zd)`\n",
				STRVIEW_PRINTF_ARG(dep_name),
				res->names[index],
				deps->count
			);

			membuf_t deps_membuf = membuf_wrap(deps->deps);
			membuf_delete_swap_tail(deps_membuf, dep_index, &deps->count);

			return;
		}
	}

	ASSERT_FAIL("Dependency not found");
}

const void* lodge_res_get_depend(struct lodge_res *res, strview_t name, struct lodge_res_handle dependency)
{
	// TODO(TS): need to check if this res is already a dependency (only once to not trigger reload multiple times)
	const void* data = lodge_res_get(res, name);
	ASSERT(data);

	const int64_t index = lodge_res_find_by_name(res, name);
	if(index < 0) {
		ASSERT_FAIL("Asset not found");
		return NULL;
	}

	lodge_res_append_dep(res, (size_t)index, dependency);

	return data;
}

void lodge_res_release_depend(struct lodge_res *res, strview_t name, struct lodge_res_handle dependency)
{
	const int64_t index = lodge_res_find_by_name(res, name);
	if(index < 0) {
		ASSERT_FAIL("Asset not found");
		return;
	}

	lodge_res_release_index(res, (size_t)index);
	lodge_res_remove_dep(res, (size_t)index, dependency);
}

//
// Removes all this dependency from all assets in the asset manager that may be depending on it.
//
void lodge_res_clear_dependency(struct lodge_res *res, struct lodge_res_handle dependency)
{
	//
	// TODO(TS): should have a reverse dependency lookup for this instead
	//

	for(uint32_t res_index = 0; res_index < res->count; res_index++) {
		struct lodge_res_deps *deps = &res->deps[res_index];
		
		for(size_t dep_index = 0; dep_index < deps->count; dep_index++) {
			if(lodge_res_handle_equals(&deps->deps[dep_index], &dependency)) {
				lodge_res_release_index(res, (size_t)res_index);
				lodge_res_remove_dep(res, (size_t)res_index, dependency);

				//
				// NOTE(TS): `lodge_res_remove_dep()` will delete_swap_tail
				//
				dep_index--;
			}
		}
	}
}

void lodge_res_set_userdata(struct lodge_res *res, size_t index, void *userdata)
{
	ASSERT(index < LODGE_RES_USERDATA_MAX);
	res->userdata[index] = userdata;
}

void* lodge_res_get_userdata(struct lodge_res *res, size_t index)
{
	ASSERT(index < LODGE_RES_USERDATA_MAX);
	return res->userdata[index];
}

strview_t lodge_res_id_to_name(struct lodge_res *res, lodge_res_id_t id)
{
	ASSERT(lodge_res_id_is_valid(id));

	for(size_t i = 0, count = res->count; i < count; i++) {
		if(res->ids[i] == id) {
			return strbuf_to_strview(strbuf_wrap(res->names[i]));
		}
	}
	
	ASSERT_FAIL("Resource ID not found");
	return strview_static("n/a");
}

lodge_res_id_t lodge_res_name_to_id(struct lodge_res *res, strview_t name)
{
	for(size_t i = 0, count = res->count; i < count; i++) {
		if(strview_equals(strview_wrap(res->names[i]), name)) {
			return res->ids[i];
		}
	}
	
	ASSERT_FAIL("Resource name not found");
	return lodge_res_id_make_invalid();
}

strview_t lodge_res_handle_find_name(struct lodge_res_handle *handle)
{
	return lodge_res_id_to_name(handle->resources, handle->id);
}

lodge_res_id_t lodge_res_id_make_invalid()
{
	return 0;
}

lodge_res_id_t lodge_res_id_make(uint32_t hash)
{
	return (uint64_t)LODGE_BIT(64) | (uint64_t)hash;
}

bool lodge_res_id_is_valid(lodge_res_id_t id)
{
	// Get 1st bit
	return id >> 63;
}

uint32_t lodge_res_id_get_hash(lodge_res_id_t id)
{
	// Get lower 32 bits
	return (uint32_t)id;
}

uint32_t lodge_res_id_get_reserved_uint31(lodge_res_id_t id)
{
	// Remove hash + valid bit
	return (id >> 32) & 0x7FFFFFFF;
}