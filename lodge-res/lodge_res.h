#ifndef _LODGE_RES_H
#define _LODGE_RES_H

#include "strview.h"

#include <stdint.h>
#include <stdbool.h>

struct lodge_res;

// Resource ID type.
//
// Designed so that the literal `0` will always be an invalid Resource ID, and
// if two IDs compare equal they will be referring to the same resource.
//
// Bit layout:
//
//		[64..64]		Valid flag: true/false.
//		[63..31]		Reserved for future implementation of unique counter.
//		[32..00]		Name hash.
//
typedef uint64_t lodge_res_id_t;

#define LODGE_RES_ID_FMT "{ %u (valid: %d, hash: %u) }"

#define LODGE_RES_ID_ARG(res_id) lodge_res_id_is_valid(res_id), lodge_res_id_get_hash(res_id)

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

lodge_res_id_t				lodge_res_id_make_invalid();
lodge_res_id_t				lodge_res_id_make(uint32_t hash);
bool						lodge_res_id_is_valid(lodge_res_id_t id);
uint32_t					lodge_res_id_get_hash(lodge_res_id_t id);
uint32_t					lodge_res_id_get_reserved_uint31(lodge_res_id_t id);

#endif
