#ifndef _LODGE_RES_H
#define _LODGE_RES_H

#include "strview.h"

#include <stdint.h>
#include <stdbool.h>

struct lodge_res;

//
// NOTE(TS): maybe use uint64_t and store flags in upper bits:
//		- Valid handle (want to return 0 from utility functions to signal invalid)
//		- Unique number (sanity check for hash collision?)
//
typedef uint32_t lodge_res_id_t;

#define LODGE_RES_ID_FMT "%u"

// TODO(TS): should replace this with a hash; name/id can be fetched via lodge_res_handle_to_name()
// this is due to the strview potentially being garbage ptr depending on what happens in *resources
struct lodge_res_handle
{
	lodge_res_id_t			id;
	struct lodge_res		*resources;
};

struct lodge_res_desc
{
	strview_t				name;
	size_t					size;

	bool					(*new_inplace)(struct lodge_res *res, strview_t name, lodge_res_id_t id, void *data, size_t data_size);
	bool					(*reload_inplace)(struct lodge_res *res, strview_t name, lodge_res_id_t id, void *data);
	void					(*free_inplace)(struct lodge_res *res, strview_t name, lodge_res_id_t id, void *data);
};

void						lodge_res_new_inplace(struct lodge_res *res, struct lodge_res_desc desc);
void						lodge_res_free_inplace(struct lodge_res *res);
size_t						lodge_res_sizeof();

struct lodge_res*			lodge_res_new(struct lodge_res_desc desc);
void						lodge_res_free(struct lodge_res *res);

const void*					lodge_res_get(struct lodge_res *res, strview_t name);
void						lodge_res_release(struct lodge_res *res, strview_t name);
void						lodge_res_reload(struct lodge_res *res, strview_t name);

const void*					lodge_res_get_depend(struct lodge_res *res, strview_t name, struct lodge_res_handle dependency);
void						lodge_res_release_depend(struct lodge_res *res, strview_t name, struct lodge_res_handle dependency);
void						lodge_res_clear_dependency(struct lodge_res *res, struct lodge_res_handle dependency);

void						lodge_res_set_userdata(struct lodge_res *res, size_t index, void *userdata);
void*						lodge_res_get_userdata(struct lodge_res *res, size_t index);

strview_t					lodge_res_id_to_name(struct lodge_res *res, lodge_res_id_t id);
lodge_res_id_t				lodge_res_name_to_id(struct lodge_res *res, strview_t name);

strview_t					lodge_res_handle_find_name(struct lodge_res_handle *handle);

#endif
