#ifndef _LODGE_VARIANT_H
#define _LODGE_VARIANT_H

#include "math4.h"
#include "lodge_type.h"

#include <stdint.h>
#include <stdbool.h>

//
// TODO(TS): store strings in variants
//
//		- Some conceptual challenges: is string a separate LODGE_TYPE_STRING?
//		- Or is it an array of LODGE_TYPE_CHAR?
//		- Size of the string/char-array is not fixed -- current lodge_type design requires fixed size types
//		- Maybe all variants can hold arrays of the basic type?
//		- lodge_variant_set_string(strview_static("hello")) => lodge_variant_set_type_array(LODGE_TYPE_CHAR, 5, &"hello")
//		- Variants must all of a sudden contain an array_size parameter -- bleh! `size_t` is best but bloats size of lodge_variant for non-array types
//		- OR: use lodge_type_funcs to register constructor and destructor for types?
//			lodge_variant_set_type(LODGE_TYPE_STRING, "hello");
//			lodge_variant_on_type_created() => heap_data = strcpy("hello")
//			lodge_variant_on_type_destroy() => free(heap_data)
//		- This could also work other types, refcounted etc
//		- Also will probably need array_t and friends to live in variants.
//

struct lodge_variant
{
	lodge_type_t				type;

	union
	{
		bool					boolean;
		uint32_t				u32;
		uint64_t				u64;
		int32_t					i32;
		int64_t					i64;
		float					f32;
		double					f64;
		vec2					vec2;
		vec3					vec3;
		vec4					vec4;
		mat4					mat4;
		void					*data_heap;
		char					data_inline[sizeof(mat4)];
	};
};

void							lodge_variant_reset(struct lodge_variant *variant);
bool							lodge_variant_is_set(const struct lodge_variant *variant);

struct lodge_variant			lodge_variant_make_boolean(bool value);
struct lodge_variant			lodge_variant_make_u32(uint32_t value);
struct lodge_variant			lodge_variant_make_u64(uint64_t value);
struct lodge_variant			lodge_variant_make_i32(int32_t value);
struct lodge_variant			lodge_variant_make_i64(int64_t value);
struct lodge_variant			lodge_variant_make_f32(float value);
struct lodge_variant			lodge_variant_make_f64(double value);
struct lodge_variant			lodge_variant_make_vec2(vec2 value);
struct lodge_variant			lodge_variant_make_vec3(vec3 value);
struct lodge_variant			lodge_variant_make_vec4(vec4 value);
struct lodge_variant			lodge_variant_make_mat4(mat4 value);
struct lodge_variant			lodge_variant_make_type(lodge_type_t type, const void *ptr);

void							lodge_variant_set_boolean(struct lodge_variant *dst, bool value);
void							lodge_variant_set_u32(struct lodge_variant *dst, uint32_t value);
void							lodge_variant_set_u64(struct lodge_variant *dst, uint64_t value);
void							lodge_variant_set_i32(struct lodge_variant *dst, int32_t value);
void							lodge_variant_set_i64(struct lodge_variant *dst, int64_t value);
void							lodge_variant_set_f32(struct lodge_variant *dst, float value);
void							lodge_variant_set_f64(struct lodge_variant *dst, double value);
void							lodge_variant_set_vec2(struct lodge_variant *dst, vec2 value);
void							lodge_variant_set_vec3(struct lodge_variant *dst, vec3 value);
void							lodge_variant_set_vec4(struct lodge_variant *dst, vec4 value);
void							lodge_variant_set_mat4(struct lodge_variant *dst, mat4 value);
void							lodge_variant_set_type(struct lodge_variant *dst, lodge_type_t type, const void *src);

void							lodge_variant_copy(struct lodge_variant *dst, const struct lodge_variant *src);

const bool*						lodge_variant_get_boolean(const struct lodge_variant *dst);
const uint32_t*					lodge_variant_get_u32(const struct lodge_variant *dst);
const uint64_t*					lodge_variant_get_u64(const struct lodge_variant *dst);
const int32_t*					lodge_variant_get_i32(const struct lodge_variant *dst);
const int64_t*					lodge_variant_get_i64(const struct lodge_variant *dst);
const float*					lodge_variant_get_f32(const struct lodge_variant *dst);
const double*					lodge_variant_get_f64(const struct lodge_variant *dst);
const vec2*						lodge_variant_get_vec2(const struct lodge_variant *dst);
const vec3*						lodge_variant_get_vec3(const struct lodge_variant *dst);
const vec4*						lodge_variant_get_vec4(const struct lodge_variant *dst);
const mat4*						lodge_variant_get_mat4(const struct lodge_variant *dst);
const void*						lodge_variant_get_type(const struct lodge_variant *dst, lodge_type_t type);

const void*						lodge_variant_get_data_ptr(const struct lodge_variant *variant);
void*							lodge_variant_access_data_ptr(struct lodge_variant *variant);
#endif