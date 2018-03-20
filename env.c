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
#include "math4.h"

struct env_var* env_var_get_by_name(struct env *e, const char *name)
{
	int name_len = strnlen(name, ENV_VAR_NAME_MAX);

	for(int i=0; i<e->len; i++) {
		struct env_var *var = (struct env_var *) &(e->vars[i]);
		int var_name_len = strnlen(var->name, ENV_VAR_NAME_MAX);
		if(strncmp(var->name, name, imax(var_name_len, name_len)) == 0) {
			return var;
		}
	}

	return NULL;
}

static void env_var_set(struct env_var *var, const char *name, int type,
		void *value, size_t value_size)
{
	strncpy(var->name, name, ENV_VAR_NAME_MAX);
	var->type = type;
	var->value = value;
	var->value_size = value_size;
}

static int env_var_new(struct env *env, const char *name, int type,
		void *value, size_t value_size, struct env_var **dst)
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

int env_bind_1f(struct env *env, const char *name, float *value)
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

int env_set_1f(struct env *env, const char *name, const float value)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		env_error("Unknown variable: \"%s\"\n", name);
		return -1;
	} else if(var->type != ENV_VAR_TYPE_1F) {
		env_error("Not a float variable: \"%s\"\n", name);
		return -1;
	}

	(*((float *) var->value)) = value;
	return 0;
}

int env_bind_2f(struct env *env, const char *name, vec2 *v)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		if(env_var_new(env, name, ENV_VAR_TYPE_2F, v,
					sizeof(vec2), &var) != 0) {
			env_error("Too many variables in environment\n");
			return -1;
		}
	}

	env_var_set(var, name, ENV_VAR_TYPE_2F, v, sizeof(vec2));

	/* Utility setters. */
	char cmd_name[ENV_VAR_NAME_MAX] = { 0 };

	/* "name".x utility setter. */
	snprintf(cmd_name, ENV_VAR_NAME_MAX, "%s.x", name);
	env_bind_1f(env, cmd_name, &v->x);

	/* "name".y utility setter. */
	snprintf(cmd_name, ENV_VAR_NAME_MAX, "%s.y", name);
	env_bind_1f(env, cmd_name, &v->y);

	return 0;
}

int env_set_2f(struct env *env, const char *name, const vec2 v)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		env_error("Unknown variable: \"%s\"\n", name);
		return -1;
	} else if(var->type != ENV_VAR_TYPE_2F) {
		env_error("Not a vec2 variable: \"%s\"\n", name);
		return -1;
	}

	((float *) var->value)[0] = v.v[0];
	((float *) var->value)[1] = v.v[1];
	return 0;
}

int env_bind_3f(struct env *env, const char *name, vec3 *v)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		if(env_var_new(env, name, ENV_VAR_TYPE_3F, v,
					sizeof(vec3), &var) != 0) {
			env_error("Too many variables in environment\n");
			return -1;
		}
	}

	/* Utility setters. */
	char cmd_name[ENV_VAR_NAME_MAX] = { 0 };

	/* "name".x utility setter. */
	snprintf(cmd_name, ENV_VAR_NAME_MAX, "%s.x", name);
	env_bind_1f(env, cmd_name, &v->x);

	/* "name".y utility setter. */
	snprintf(cmd_name, ENV_VAR_NAME_MAX, "%s.y", name);
	env_bind_1f(env, cmd_name, &v->y);

	/* "name".z utility setter. */
	snprintf(cmd_name, ENV_VAR_NAME_MAX, "%s.z", name);
	env_bind_1f(env, cmd_name, &v->z);

	env_var_set(var, name, ENV_VAR_TYPE_3F, v, sizeof(vec3));
	return 0;
}

int env_set_3f(struct env *env, const char *name, const vec3 v)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		env_error("Unknown variable: \"%s\"\n", name);
		return -1;
	} else if(var->type != ENV_VAR_TYPE_3F) {
		env_error("Not a vec3 variable: \"%s\"\n", name);
		return -1;
	}

	((float *) var->value)[0] = v.v[0];
	((float *) var->value)[1] = v.v[1];
	((float *) var->value)[2] = v.v[2];
	return 0;
}

int env_bind_bool(struct env *env, const char *name, int *value)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		if(env_var_new(env, name, ENV_VAR_TYPE_BOOL, value,
					sizeof(int), &var) != 0) {
			env_error("Too many variables in environment\n");
			return -1;
		}
	}

	env_var_set(var, name, ENV_VAR_TYPE_BOOL, value, sizeof(int));
	return 0;
}

int env_set_bool(struct env *env, const char *name, const int value)
{
	struct env_var *var = env_var_get_by_name(env, name);

	if(var == NULL) {
		env_error("Unknown variable: \"%s\"\n", name);
		return -1;
	} else if(var->type != ENV_VAR_TYPE_BOOL) {
		env_error("Not a boolean variable: \"%s\"\n", name);
		return -1;
	}

	(*((int *) var->value)) = value;
	return 0;
}
