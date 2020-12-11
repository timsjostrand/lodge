#include "lodge_variant.h"

#include "lodge_platform.h"
#include "lodge_assert.h"

#include <string.h>

static inline bool lodge_variant_is_type_heap_allocated(lodge_type_t type)
{
	return lodge_type_get_size(type) >= sizeof_member(struct lodge_variant, data_inline);
}

void lodge_variant_reset(struct lodge_variant *variant)
{
	if(lodge_variant_is_type_heap_allocated(variant->type)) {
		free(variant->data_heap);
	}
	variant->type = LODGE_TYPE_NONE;
}

bool lodge_variant_is_set(const struct lodge_variant *variant)
{
	return variant->type != LODGE_TYPE_NONE;
}

#define lodge_variant_make_impl( C_TYPE, UNION_FIELD, ENUM_TYPE ) \
	struct lodge_variant lodge_variant_make_ ## UNION_FIELD ( C_TYPE value ) \
	{ \
		return (struct lodge_variant) { \
			.type = ENUM_TYPE , \
			.UNION_FIELD = value, \
		}; \
	}

lodge_variant_make_impl(bool, boolean, LODGE_TYPE_BOOL);
lodge_variant_make_impl(uint32_t, u32, LODGE_TYPE_U32);
lodge_variant_make_impl(uint64_t, u64, LODGE_TYPE_U64);
lodge_variant_make_impl(int32_t, i32, LODGE_TYPE_I32);
lodge_variant_make_impl(int64_t, i64, LODGE_TYPE_I64);
lodge_variant_make_impl(float, f32, LODGE_TYPE_F32);
lodge_variant_make_impl(double, f64, LODGE_TYPE_F64);
lodge_variant_make_impl(vec2, vec2, LODGE_TYPE_VEC2);
lodge_variant_make_impl(vec3, vec3, LODGE_TYPE_VEC3);
lodge_variant_make_impl(vec4, vec4, LODGE_TYPE_VEC4);
lodge_variant_make_impl(mat4, mat4, LODGE_TYPE_MAT4);

struct lodge_variant lodge_variant_make_type(lodge_type_t type, const void *src)
{
	struct lodge_variant tmp = { 0 };
	lodge_variant_set_type(&tmp, type, src);
	return tmp;
}

#undef lodge_variant_make_impl

static void lodge_variant_set_type_inline(struct lodge_variant *dst, lodge_type_t type, const void *src)
{
	ASSERT(lodge_type_get_size(type) <= sizeof_member(struct lodge_variant, data_inline));
	lodge_variant_reset(dst);
	dst->type = type;
	memcpy(lodge_variant_access_data_ptr(dst), src, lodge_type_get_size(type));
}

static void lodge_variant_set_type_heap(struct lodge_variant *dst, lodge_type_t type, const void *src)
{
	//
	// TODO(TS): optimize: only realloc if grow needed, otherwise reuse old heap
	//

	ASSERT(lodge_type_get_size(type) > sizeof_member(struct lodge_variant, data_inline));
	lodge_variant_reset(dst);
	dst->type = type;
	
	size_t type_size = lodge_type_get_size(type);
	dst->data_heap = malloc(type_size);
	ASSERT(dst->data_heap);
	memcpy(dst->data_heap, src, type_size);
}

#define lodge_variant_set_impl(C_TYPE, UNION_FIELD, ENUM_TYPE) \
	void lodge_variant_set_ ## UNION_FIELD(struct lodge_variant *dst, C_TYPE value) \
	{ \
		lodge_variant_reset(dst); \
		dst->type = ENUM_TYPE; \
		dst->UNION_FIELD = value; \
	}

lodge_variant_set_impl(bool, boolean, LODGE_TYPE_BOOL);
lodge_variant_set_impl(uint32_t, u32, LODGE_TYPE_U32);
lodge_variant_set_impl(uint64_t, u64, LODGE_TYPE_U64);
lodge_variant_set_impl(int32_t, i32, LODGE_TYPE_I32);
lodge_variant_set_impl(int64_t, i64, LODGE_TYPE_I64);
lodge_variant_set_impl(float, f32, LODGE_TYPE_F32);
lodge_variant_set_impl(double, f64, LODGE_TYPE_F64);
lodge_variant_set_impl(vec2, vec2, LODGE_TYPE_VEC2);
lodge_variant_set_impl(vec3, vec3, LODGE_TYPE_VEC3);
lodge_variant_set_impl(vec4, vec4, LODGE_TYPE_VEC4);
lodge_variant_set_impl(mat4, mat4, LODGE_TYPE_MAT4);

void lodge_variant_set_type(struct lodge_variant *dst, lodge_type_t type, const void *src)
{
	if(lodge_variant_is_type_heap_allocated(type)) {
		lodge_variant_set_type_heap(dst, type, src);
	} else {
		lodge_variant_set_type_inline(dst, type, src);
	}
}

#undef lodge_variant_set_impl

void lodge_variant_copy(struct lodge_variant *dst, const struct lodge_variant *src)
{
	lodge_variant_reset(dst);
	lodge_variant_set_type(dst, src->type, &src->data_inline[0]);
}

const bool* lodge_variant_get_boolean(const struct lodge_variant *dst)
{
	return (dst && dst->type == LODGE_TYPE_BOOL) ? &dst->boolean : NULL;
}

const uint32_t*	lodge_variant_get_u32(const struct lodge_variant *dst)
{
	return (dst && dst->type == LODGE_TYPE_U32) ? &dst->u32 : NULL;
}

const uint64_t*	lodge_variant_get_u64(const struct lodge_variant *dst)
{
	return (dst && dst->type == LODGE_TYPE_U64) ? &dst->u64 : NULL;
}

const int32_t* lodge_variant_get_i32(const struct lodge_variant *dst)
{
	return (dst && dst->type == LODGE_TYPE_I32) ? &dst->i32 : NULL;
}

const int64_t* lodge_variant_get_i64(const struct lodge_variant *dst)
{
	return (dst && dst->type == LODGE_TYPE_I64) ? &dst->i64 : NULL;
}

const float* lodge_variant_get_f32(const struct lodge_variant *dst)
{
	return (dst && dst->type == LODGE_TYPE_F32) ? &dst->f32 : NULL;
}

const double* lodge_variant_get_f64(const struct lodge_variant *dst)
{
	return (dst && dst->type == LODGE_TYPE_F64) ? &dst->f64 : NULL;
}

const vec2*	lodge_variant_get_vec2(const struct lodge_variant *dst)
{
	return (dst && dst->type == LODGE_TYPE_VEC2) ? &dst->vec2 : NULL;
}

const vec3*	lodge_variant_get_vec3(const struct lodge_variant *dst)
{
	return (dst && dst->type == LODGE_TYPE_VEC3) ? &dst->vec3 : NULL;
}

const vec4*	lodge_variant_get_vec4(const struct lodge_variant *dst)
{
	return (dst && dst->type == LODGE_TYPE_VEC4) ? &dst->vec4 : NULL;
}

const mat4* lodge_variant_get_mat4(const struct lodge_variant *dst)
{
	return (dst && dst->type == LODGE_TYPE_MAT4) ? &dst->mat4 : NULL;
}

const void* lodge_variant_get_type(const struct lodge_variant *dst, lodge_type_t type)
{
	ASSERT(type != NULL);
	if(!dst) {
		return NULL;
	}
	if(dst->type != type) {
		return NULL;
	}
	if(lodge_variant_is_type_heap_allocated(dst->type)) {
		return dst->data_heap;
	} else {
		return &dst->data_inline;
	}
}

const void* lodge_variant_get_data_ptr(const struct lodge_variant *variant)
{
	return lodge_variant_access_data_ptr((struct lodge_variant *)variant);
}

void* lodge_variant_access_data_ptr(struct lodge_variant *variant)
{
	if(lodge_variant_is_type_heap_allocated(variant->type)) {
		return variant->data_heap;
	} else {
		return &variant->data_inline[0];
	}
}