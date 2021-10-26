#ifndef _LODGE_ARGV_H
#define _LODGE_ARGV_H

#include "log.h"
#include "strview.h"

#include <stdint.h>
#include <stdbool.h>

enum lodge_arg_type
{
	LODGE_ARG_TYPE_FLAG,
	LODGE_ARG_TYPE_POSITIONAL,
	LODGE_ARG_TYPE_KEY_VALUE,
};

struct lodge_arg
{
	enum lodge_arg_type				type;
	uint32_t						key_hash;
	strview_t						key;
	strview_t						value;
};

struct lodge_argv_positional
{
	strview_t						key;
	uint32_t						key_hash;
};

struct lodge_argv_positionals
{
	size_t							count;
	struct lodge_argv_positional	elements[256];
};

struct lodge_argv
{
	strview_t						path;

	uint32_t						count;
	struct lodge_arg				elements[256];

	struct lodge_argv_positionals	positionals;
};

void								lodge_argv_new_inplace(struct lodge_argv *dst, int argc, char **argv);
void								lodge_argv_free_inplace(struct lodge_argv *dst);

bool								lodge_argv_is_arg(const struct lodge_argv *dst, strview_t key);
const struct lodge_arg*				lodge_argv_get_arg_by_hash(const struct lodge_argv *dst, uint32_t key_hash);
const struct lodge_arg*				lodge_argv_get_arg_by_name(const struct lodge_argv *dst, strview_t key);

strview_t							lodge_argv_get_str(const struct lodge_argv *argv, strview_t key, strview_t default_value);
bool								lodge_argv_get_bool(const struct lodge_argv *argv, strview_t key, bool default_value);
uint32_t							lodge_argv_get_u32(const struct lodge_argv *argv, strview_t key, uint32_t default_value);
uint64_t							lodge_argv_get_u64(const struct lodge_argv *argv, strview_t key, uint64_t default_value);

const struct lodge_argv_positional*	lodge_argv_positional_it_begin(const struct lodge_argv *argv);
const struct lodge_argv_positional*	lodge_argv_positional_it_next(const struct lodge_argv *argv, const struct lodge_argv_positional *it);

#endif
