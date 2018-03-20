#ifndef _ENV_H
#define _ENV_H

#include "log.h"
#include "math4.h"

#define env_debug(...) debugf("Env", __VA_ARGS__)
#define env_error(...) errorf("Env", __VA_ARGS__)

#define ENV_MAX					255 /* Number of environment variables. */
#define ENV_VAR_NAME_MAX		255
#define ENV_VAR_TYPE_UNKNOWN	0
#define ENV_VAR_TYPE_STR		1
#define ENV_VAR_TYPE_1F			2
#define ENV_VAR_TYPE_2F			3
#define ENV_VAR_TYPE_3F			4
#define ENV_VAR_TYPE_BOOL		5

struct env_var {
	char		name[ENV_VAR_NAME_MAX];		/* Variable name. */
	void		*value;						/* Variable value. */
	int			type;						/* The type of data pointed to by .value: one of ENV_VAR_TYPE_*. */
	size_t		value_size;					/* The size in bytes of the value data. */
};

struct env {
	struct env_var	vars[ENV_MAX];
	int				len;
};

struct env_var* env_var_get_by_name(struct env *e, const char *name);

int  env_bind_1f(struct env *env, const char *name, float *value);
int  env_set_1f(struct env *env, const char *name, const float value);

int  env_bind_2f(struct env *env, const char *name, vec2 *v);
int  env_set_2f(struct env *env, const char *name, const vec2 v);

int  env_bind_3f(struct env *env, const char *name, vec3 *v);
int  env_set_3f(struct env *env, const char *name, const vec3 v);

int  env_bind_bool(struct env *env, const char *name, int *value);
int  env_set_bool(struct env *env, const char *name, const int value);

#endif
