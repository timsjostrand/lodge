/**
 * Variable environment.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "env.h"
#include "str.h"
#include "log.h"

#define env_debug(...) debugf("Env", __VA_ARGS__)
#define env_error(...) errorf("Env", __VA_ARGS__)

struct env_var* env_var_get_by_name(struct env *e, const strview_t name)
{
	for(int i=0; i<e->len; i++) {
		struct env_var *var = (struct env_var *) &(e->vars[i]);
		const strview_t var_name = strbuf_to_strview(strbuf_wrap(var->name));
		if(strview_equals(var_name, name)) {
			return var;
		}
	}

	return NULL;
}

static void env_var_set(struct env_var *var, const strview_t name, int type, void *value, size_t value_size)
{
	strbuf_set(strbuf_wrap(var->name), name);
	var->type = type;
	var->value = value;
	var->value_size = value_size;
}

static int env_var_new(struct env *env, const strview_t name, int type, void *value, size_t value_size, struct env_var **dst)
{
	if(env->len >= ENV_MAX) {
		return -1;
	}
	struct env_var *var = &(env->vars[env->len]);
	env_var_set(var, name, type, value, value_size);
	(*dst) = var;
	env->len ++;
	return 0;
}

int env_bind_float(struct env *env, const strview_t name, float *value)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		if(env_var_new(env, name, ENV_VAR_TYPE_1F, value,
					sizeof(float), &var) != 0) {
			env_error("Too many variables in environment\n");
			return -1;
		}
	}

	env_var_set(var, name, ENV_VAR_TYPE_1F, value, sizeof(float));
	return 0;
}

int env_set_float(struct env *env, const strview_t name, const float value)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		env_error("Unknown variable: \"" STRVIEW_PRINTF_FMT "\"\n", STRVIEW_PRINTF_ARG(name));
		return -1;
	} else if(var->type != ENV_VAR_TYPE_1F) {
		env_error("Not a float variable: \"" STRVIEW_PRINTF_FMT "\"\n", STRVIEW_PRINTF_ARG(name));
		return -1;
	}

	(*((float *) var->value)) = value;
	return 0;
}

int env_bind_vec2(struct env *env, const strview_t name, vec2 *v)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		if(env_var_new(env, name, ENV_VAR_TYPE_2F, v, sizeof(vec2), &var) != 0) {
			env_error("Too many variables in environment\n");
			return -1;
		}
	}

	env_var_set(var, name, ENV_VAR_TYPE_2F, v, sizeof(vec2));

	/* Utility setters. */
	char cmd_name[ENV_VAR_NAME_MAX] = { 0 };
	strbuf_t cmd_name_buf = strbuf_wrap(cmd_name);

	/* "name".x utility setter. */
	strbuf_setf(cmd_name_buf, STRVIEW_PRINTF_FMT ".x", STRVIEW_PRINTF_ARG(name));
	env_bind_float(env, strbuf_to_strview(cmd_name_buf), &v->x);

	/* "name".y utility setter. */
	strbuf_setf(cmd_name_buf, STRVIEW_PRINTF_FMT ".y", STRVIEW_PRINTF_ARG(name));
	env_bind_float(env, strbuf_to_strview(cmd_name_buf), &v->y);

	return 0;
}

int env_set_vec2(struct env *env, const strview_t name, const vec2 v)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		env_error("Unknown variable: \"" STRVIEW_PRINTF_FMT "\"\n", STRVIEW_PRINTF_ARG(name));
		return -1;
	} else if(var->type != ENV_VAR_TYPE_2F) {
		env_error("Not a vec2 variable: \"" STRVIEW_PRINTF_FMT "\"\n", STRVIEW_PRINTF_ARG(name));
		return -1;
	}

	((float *) var->value)[0] = v.v[0];
	((float *) var->value)[1] = v.v[1];
	return 0;
}

int env_bind_vec3(struct env *env, const strview_t name, vec3 *v)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		if(env_var_new(env, name, ENV_VAR_TYPE_3F, v, sizeof(vec3), &var) != 0) {
			env_error("Too many variables in environment\n");
			return -1;
		}
	}

	/* Utility setters. */
	char cmd_name[ENV_VAR_NAME_MAX] = { 0 };
	strbuf_t cmd_name_buf = strbuf_wrap(cmd_name);

	/* "name".x utility setter. */
	strbuf_setf(cmd_name_buf, STRVIEW_PRINTF_FMT ".x", STRVIEW_PRINTF_ARG(name));
	env_bind_float(env, strbuf_to_strview(cmd_name_buf), &v->x);

	/* "name".y utility setter. */
	strbuf_setf(cmd_name_buf, STRVIEW_PRINTF_FMT ".y", STRVIEW_PRINTF_ARG(name));
	env_bind_float(env, strbuf_to_strview(cmd_name_buf), &v->y);

	/* "name".z utility setter. */
	strbuf_setf(cmd_name_buf, STRVIEW_PRINTF_FMT ".z", STRVIEW_PRINTF_ARG(name));
	env_bind_float(env, strbuf_to_strview(cmd_name_buf), &v->z);

	env_var_set(var, name, ENV_VAR_TYPE_3F, v, sizeof(vec3));
	return 0;
}

int env_set_vec3(struct env *env, const strview_t name, const vec3 v)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		env_error("Unknown variable: \"" STRVIEW_PRINTF_FMT "\"\n", STRVIEW_PRINTF_ARG(name));
		return -1;
	} else if(var->type != ENV_VAR_TYPE_3F) {
		env_error("Not a vec3 variable: \"" STRVIEW_PRINTF_FMT "\"\n", STRVIEW_PRINTF_ARG(name));
		return -1;
	}

	((float *) var->value)[0] = v.v[0];
	((float *) var->value)[1] = v.v[1];
	((float *) var->value)[2] = v.v[2];
	return 0;
}

int env_bind_bool(struct env *env, const strview_t name, int *value)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		if(env_var_new(env, name, ENV_VAR_TYPE_BOOL, value, sizeof(int), &var) != 0) {
			env_error("Too many variables in environment\n");
			return -1;
		}
	}

	env_var_set(var, name, ENV_VAR_TYPE_BOOL, value, sizeof(int));
	return 0;
}

int env_set_bool(struct env *env, const strview_t name, const int value)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		env_error("Unknown variable: \"" STRVIEW_PRINTF_FMT "\"\n", STRVIEW_PRINTF_ARG(name));
		return -1;
	} else if(var->type != ENV_VAR_TYPE_BOOL) {
		env_error("Not a boolean variable: \"" STRVIEW_PRINTF_FMT "\"\n", STRVIEW_PRINTF_ARG(name));
		return -1;
	}

	(*((int *) var->value)) = value;
	return 0;
}
