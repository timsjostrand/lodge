#ifndef _LODGE_RES_H
#define _LODGE_RES_H

#include "strview.h"

#include <stdint.h>
#include <stdbool.h>

struct lodge_res;

struct lodge_res_handle
{
	struct lodge_res		*resources;
	strview_t				id;
};

struct lodge_res_desc
{
	strview_t				name;
	size_t					size;

	bool					(*new_inplace)(struct lodge_res *res, strview_t name, void *data, size_t data_size);
	bool					(*reload_inplace)(struct lodge_res *res, strview_t name, void *data);
	void					(*free_inplace)(struct lodge_res *res, strview_t name, void *data);
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

void						lodge_res_set_userdata(struct lodge_res *res, size_t index, void *userdata);
void*						lodge_res_get_userdata(struct lodge_res *res, size_t index);

#endif
