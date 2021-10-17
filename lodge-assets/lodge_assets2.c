#include "lodge_assets2.h"

#include "str.h"
#include "dynbuf.h"
#include "sparse_set.h"
#include "strbuf.h"

#include "lodge_platform.h"

#include <stdbool.h>

struct lodge_asset_listener
{
	struct lodge_assets2			*assets;
	lodge_asset_t					asset;
};

//
// TODO(TS): this may be very expensive memory/heap-wise, and most
// assets will have 0-1 listeners...
// 
// 
// It would be nice to have a "hybridbuf" that can take a dynbuf-like interface:
// 
//		struct lodge_asset_listeners
//		{
//			size_t							count;
//			size_t							capacity;
//			struct lodge_asset_listener		elements[2];
//		};
//
// And use the stack memory while capacity < LODGE_ARRAYSIZE(elements).
// If we go over, heap-allocate and reuse elements as a ptr to heap.
// 
// Need to check that sizeof(elements) >= sizeof(void*).
//
struct lodge_asset_listeners
{
	size_t							count;
	size_t							capacity;
	struct lodge_asset_listener		*elements;
};

struct lodge_asset_info
{
	lodge_asset_t					id;
	char							name[256];
	uint32_t						name_hash;

	struct lodge_asset_listeners	listeners;
};

struct lodge_asset_infos
{
	size_t							count;
	size_t							capacity;
	struct lodge_asset_info			*elements;
};

struct lodge_assets2
{
	struct lodge_assets2_desc		desc;
	sparse_set_t					set;
	struct lodge_asset_infos		infos;
	void							*userdatas[16];
};

static uint32_t lodge_asset_to_index(const lodge_asset_t asset)
{
	ASSERT(asset != NULL);
	return (uint32_t)((uintptr_t)asset - 1);
}

static lodge_asset_t lodge_asset_from_index(uint32_t index)
{
	return (struct lodge_asset*)((uintptr_t)index + 1);
}

static struct lodge_asset_info* lodge_assets2_get_info_by_index(struct lodge_assets2 *assets, uint32_t index)
{
	ASSERT_OR(assets && index < assets->infos.count) { return NULL; }
	return &assets->infos.elements[index];
}

static struct lodge_asset_info* lodge_asset_to_info(struct lodge_assets2 *assets, lodge_asset_t asset)
{
	uint32_t index = lodge_asset_to_index(asset);
	return lodge_assets2_get_info_by_index(assets, index);
}

static void lodge_assets2_invalidate_listeners(struct lodge_assets2 *assets, struct lodge_asset_info *info)
{
	for(size_t i = 0, count = info->listeners.count; i < count; i++) {
		struct lodge_asset_listener *listener = &info->listeners.elements[i];
		lodge_assets2_invalidate(listener->assets, listener->asset);
	}
}

static void lodge_asset_info_new_inplace(struct lodge_asset_info *info, lodge_asset_t asset, strview_t name)
{
	info->id = asset;
	strbuf_set(strbuf(info->name), name);
	info->name_hash = strview_calc_hash(name);
	dynbuf_new_inplace(dynbuf(info->listeners), 1);
}

void lodge_assets2_new_inplace(struct lodge_assets2 *assets, struct lodge_assets2_desc *desc)
{
	assets->desc = *desc;
	assets->set = sparse_set_new(desc->size, 256, 256);
	dynbuf_new_inplace(dynbuf(assets->infos), 256);
}

void lodge_assets2_free_inplace(struct lodge_assets2 *assets)
{
	if(assets->desc.free_inplace) {
		for(void *it = sparse_set_it_begin(assets->set); it; it = sparse_set_it_next(assets->set, it)) {
			uint32_t index = sparse_set_get_index(assets->set, it);
			struct lodge_asset_info *info = lodge_assets2_get_info_by_index(assets, index);
			assets->desc.free_inplace(assets, strview_wrap(info->name), info->id, it);
		}
	}
	dynbuf_free_inplace(dynbuf(assets->infos));
	sparse_set_free(assets->set);
}

size_t lodge_assets2_sizeof()
{
	return sizeof(struct lodge_assets2);
}

lodge_asset_t lodge_assets2_register(struct lodge_assets2 *assets, strview_t name)
{
	const uint32_t name_hash = strview_calc_hash(name);
	lodge_asset_t tmp = lodge_assets2_find_by_name_hash(assets, name_hash);
	if(tmp) {
		return tmp;
	}

	const size_t index = assets->infos.count;
	ASSERT_OR(index < UINT32_MAX) { return NULL; }
	//
	// TODO(TS): should insert sorted by `name_hash` so we can binary search for names
	//
	struct lodge_asset_info *info = dynbuf_append_no_init(dynbuf(assets->infos));
	lodge_asset_info_new_inplace(info, lodge_asset_from_index(index), name);

	return info->id;
}

lodge_asset_t lodge_assets2_find_by_name(struct lodge_assets2 *assets, strview_t name)
{
	return lodge_assets2_find_by_name_hash(assets, strview_calc_hash(name));
}

lodge_asset_t lodge_assets2_find_by_name_hash(struct lodge_assets2 *assets, uint32_t name_hash)
{
	ASSERT_OR(assets) { return NULL; }
	//
	// TODO(TS): this should be bsearch instead
	//
	for(size_t i = 0, count = assets->infos.count; i < count; i++) {
		struct lodge_asset_info *info = &assets->infos.elements[i];
		if(info->name_hash == name_hash) {
			return info->id;
		}
	}
	return NULL;
}

void* lodge_assets2_get(struct lodge_assets2 *assets, lodge_asset_t asset)
{
	ASSERT_OR(assets) { return NULL; }
	if(!asset) { return NULL; }

	const uint32_t asset_index = lodge_asset_to_index(asset);

	//
	// Already loaded?
	//
	{
		void *data = sparse_set_get(assets->set, asset_index);
		if(data) {
			return data;
		}
	}

	//
	// Try to load asset
	//
	{
		ASSERT_OR(assets->desc.new_inplace) { return NULL; }

		struct lodge_asset_info *info = &assets->infos.elements[asset_index];

		void *data = sparse_set_set_init_zero(assets->set, asset_index);
		const bool ret = assets->desc.new_inplace(assets, strview_wrap(info->name), asset, data);
		ASSERT(ret);

		//
		// TODO(TS): fundamentally, we want to invalidate listeners here -- but, more than
		// likely that the reason why we are calling `_get()` is because that listener is 
		// about to load this asset.
		// 
		//if(ret) {
		//	lodge_assets2_invalidate_listeners(assets, info);
		//}

		return ret ? data : NULL;
	}
}


strview_t lodge_assets2_get_name(struct lodge_assets2 *assets, const lodge_asset_t asset)
{
	struct lodge_asset_info *info = asset ? lodge_assets2_get_info_by_index(assets, lodge_asset_to_index(asset)) : NULL;
	return info ? strview_wrap(info->name) : strview_null();
}

void lodge_assets2_set_userdata(struct lodge_assets2 *assets, int userdata_index, void *userdata)
{
	ASSERT_OR(userdata_index < LODGE_ARRAYSIZE(assets->userdatas)) { return; }
	assets->userdatas[userdata_index] = userdata;
}

void* lodge_assets2_get_userdata(struct lodge_assets2 *assets, int userdata_index)
{
	ASSERT_OR(userdata_index < LODGE_ARRAYSIZE(assets->userdatas)) { return NULL; }
	return assets->userdatas[userdata_index];
}

lodge_asset_t lodge_assets2_it_begin(struct lodge_assets2 *assets)
{
	return (assets->infos.count > 0) ? lodge_asset_from_index(0) : NULL;
}

lodge_asset_t lodge_assets2_it_next(struct lodge_assets2 *assets, lodge_asset_t it)
{
	const uint32_t index = it ? lodge_asset_to_index(it) + 1 : 0;
	return (index < assets->infos.count) ? lodge_asset_from_index(index) : NULL;
}

lodge_asset_t lodge_assets2_make_default(struct lodge_assets2 *assets)
{
	ASSERT_OR(assets) { return NULL; }

	lodge_asset_t id = lodge_assets2_register(assets, strview(""));

	const uint32_t index = lodge_asset_to_index(id);
	struct lodge_asset_info *info = &assets->infos.elements[index];
	strbuf_setf(strbuf(info->name), "Unnamed %u", index);
	info->name_hash = strview_calc_hash(strview_wrap(info->name));

	void* data = sparse_set_set_init_zero(assets->set, index);

	if(assets->desc.new_default_inplace) {
		bool ret = assets->desc.new_default_inplace(assets, data);
		ASSERT_OR(ret) { return NULL; }
	 } else {
		bool ret = assets->desc.new_inplace(assets, strview_wrap(info->name), id, data);
		ASSERT_OR(ret) { return NULL; }
	}

	lodge_assets2_invalidate_listeners(assets, info);

	return id;
}

void lodge_assets2_set(struct lodge_assets2 *assets, lodge_asset_t asset, const void *src)
{
	ASSERT_OR(assets && asset && src) { return; }
	const uint32_t index = lodge_asset_to_index(asset);
	void *data = sparse_set_set(assets->set, index, src);
	lodge_assets2_invalidate_listeners(assets, lodge_assets2_get_info_by_index(assets, index));
}

void lodge_assets2_add_listener(struct lodge_assets2 *assets, lodge_asset_t asset, struct lodge_assets2 *listener_assets, lodge_asset_t listener_asset)
{
	struct lodge_asset_info *info = lodge_asset_to_info(assets, asset);
	ASSERT_OR(info) { return; }

	struct lodge_asset_listener *listener = dynbuf_append(dynbuf(info->listeners), &(struct lodge_asset_listener) {
		.assets = listener_assets,
		.asset = listener_asset,
	}, sizeof(struct lodge_asset_listener));
	ASSERT_OR(listener) { return; }
}

void lodge_assets2_remove_listener_by_name(struct lodge_assets2 *assets, strview_t name, struct lodge_assets2 *listener_assets, lodge_asset_t listener_asset)
{
	lodge_asset_t asset = lodge_assets2_find_by_name(assets, name);
	if(asset) {
		lodge_assets2_remove_listener(assets, asset, listener_assets, listener_asset);
	}
}

void lodge_assets2_remove_listener(struct lodge_assets2 *assets, lodge_asset_t asset, struct lodge_assets2 *listener_assets, lodge_asset_t listener_asset)
{
	struct lodge_asset_info *info = lodge_asset_to_info(assets, asset);
	ASSERT_OR(info) { return; }

	int64_t index = dynbuf_find(dynbuf(info->listeners), &(struct lodge_asset_listener) {
		.assets = listener_assets,
		.asset = listener_asset,
	}, sizeof(struct lodge_asset_listener));
	ASSERT_OR(index >= 0) { return; }

	bool removed = dynbuf_remove(dynbuf(info->listeners), index, 1) > 0;
	ASSERT(removed);
}

#include "log.h"

void lodge_assets2_invalidate(struct lodge_assets2 *assets, lodge_asset_t asset)
{
	ASSERT_OR(assets && asset) { return; }

	uint32_t index = lodge_asset_to_index(asset);

	struct lodge_asset_info *asset_info = lodge_assets2_get_info_by_index(assets, index);
	ASSERT_OR(asset_info) { return; }

	debugf("Assets", "Invalidating `" STRVIEW_PRINTF_FMT "::%s`...\n", STRVIEW_PRINTF_ARG(assets->desc.name), asset_info->name);

	void *asset_data = sparse_set_get(assets->set, index);
	if(asset_data) {
		assets->desc.free_inplace(assets, strview_wrap(asset_info->name), asset, asset_data);
		sparse_set_remove(assets->set, index);
	}

	lodge_assets2_invalidate_listeners(assets, asset_info);
}
