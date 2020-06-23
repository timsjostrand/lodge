#ifndef _ENV_H
#define _ENV_H

#include "strview.h"
#include "math4.h"

#define ENV_MAX					255 /* Number of environment variables. */
#define ENV_VAR_NAME_MAX		255
#define ENV_VAR_TYPE_UNKNOWN	0
#define ENV_VAR_TYPE_STR		1
#define ENV_VAR_TYPE_1F			2
#define ENV_VAR_TYPE_2F			3
#define ENV_VAR_TYPE_3F			4
#define ENV_VAR_TYPE_BOOL		5

struct env_var {
	char			name[ENV_VAR_NAME_MAX];		/* Variable name. */
	void			*value;						/* Variable value. */
	int				type;						/* The type of data pointed to by .value: one of ENV_VAR_TYPE_*. */
	size_t			value_size;					/* The size in bytes of the value data. */
};

struct env {
	struct env_var	vars[ENV_MAX];
	int				len;
};

struct env_var*		env_var_get_by_name(struct env *e, const strview_t name);

int					env_bind_float(struct env *env, const strview_t name, float *value);
int					env_set_float(struct env *env, const strview_t name, const float value);

int					env_bind_vec2(struct env *env, const strview_t name, vec2 *v);
int					env_set_vec2(struct env *env, const strview_t name, const vec2 v);

int					env_bind_vec3(struct env *env, const strview_t name, vec3 *v);
int					env_set_vec3(struct env *env, const strview_t name, const vec3 v);

int					env_bind_bool(struct env *env, const strview_t name, int *value);
int					env_set_bool(struct env *env, const strview_t name, const int value);

#endif
