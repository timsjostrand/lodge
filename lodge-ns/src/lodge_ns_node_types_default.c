#include "lodge_ns_node_types_default.h"

#include "lodge_ns_node_types_helpers.h"
#include "lodge_variant.h"
#include "lodge_ns_node_type.h"
#include "lodge_ns_graph.h"
#include "lodge_ns_node.h"

#include "lodge_assert.h"

#include <stdio.h>

lodge_ns_node_const_impl(LODGE_TYPE_BOOL, boolean, bool);
lodge_ns_node_const_impl(LODGE_TYPE_U32, u32, uint32_t);
lodge_ns_node_const_impl(LODGE_TYPE_U64, u64, uint64_t);
lodge_ns_node_const_impl(LODGE_TYPE_I32, i32, int32_t);
lodge_ns_node_const_impl(LODGE_TYPE_I64, i64, int64_t);
lodge_ns_node_const_impl(LODGE_TYPE_F32, f32, float);
lodge_ns_node_const_impl(LODGE_TYPE_F64, f64, double);
lodge_ns_node_const_impl(LODGE_TYPE_VEC2, vec2, vec2);
lodge_ns_node_const_impl(LODGE_TYPE_VEC3, vec3, vec3);
lodge_ns_node_const_impl(LODGE_TYPE_VEC4, vec4, vec4);
lodge_ns_node_const_impl(LODGE_TYPE_MAT4, mat4, mat4);

//
// Unary functions
//

//lodge_ns_node_unary_func_decl(u32, uint32_t, LODGE_TYPE_U32);
//lodge_ns_node_unary_func_decl(u64, uint64_t, LODGE_TYPE_U64);
//lodge_ns_node_unary_func_decl(i32, int32_t, LODGE_TYPE_I32);
//lodge_ns_node_unary_func_decl(i64, int64_t, LODGE_TYPE_I64);
lodge_ns_node_unary_func_decl(f32, float, LODGE_TYPE_F32);
lodge_ns_node_unary_func_decl(f64, double, LODGE_TYPE_F64);
//lodge_ns_node_unary_func_decl(vec2, vec2, LODGE_TYPE_VEC2);
//lodge_ns_node_unary_func_decl(vec3, vec3, LODGE_TYPE_VEC3);
//lodge_ns_node_unary_func_decl(vec4, vec4, LODGE_TYPE_VEC4);
//lodge_ns_node_unary_func_decl(mat4, mat4, LODGE_TYPE_MAT4);

lodge_ns_node_unary_func_impl(sin, f32, float)		{ return (float)sin((double)in); }
lodge_ns_node_unary_func_impl(cos, f32, float)		{ return (float)cos((double)in); }
lodge_ns_node_unary_func_impl(sin, f64, double)		{ return sin(in); }
lodge_ns_node_unary_func_impl(cos, f64, double)		{ return cos(in); }

//
// Binary functions
//

lodge_ns_node_binary_func_decl(boolean, bool, LODGE_TYPE_BOOL);
//lodge_ns_node_binary_func_decl(u32, uint32_t, LODGE_TYPE_U32);
//lodge_ns_node_binary_func_decl(u64, uint64_t, LODGE_TYPE_U64);
lodge_ns_node_binary_func_decl(i32, int32_t, LODGE_TYPE_I32);
//lodge_ns_node_binary_func_decl(i64, int64_t, LODGE_TYPE_I64);
lodge_ns_node_binary_func_decl(f32, float, LODGE_TYPE_F32);
lodge_ns_node_binary_func_decl(f64, double, LODGE_TYPE_F64);
//lodge_ns_node_binary_func_decl(vec2, vec2, LODGE_TYPE_VEC2);
lodge_ns_node_binary_func_decl(vec3, vec3, LODGE_TYPE_VEC3);
//lodge_ns_node_binary_func_decl(vec4, vec4, LODGE_TYPE_VEC4);
//lodge_ns_node_binary_func_decl(mat4, mat4, LODGE_TYPE_MAT4);

lodge_ns_node_binary_func_impl(and, boolean, bool)	{ return lhs & rhs; }
lodge_ns_node_binary_func_impl(or, boolean, bool)	{ return lhs | rhs; }
lodge_ns_node_binary_func_impl(xor, boolean, bool)	{ return lhs ^ rhs; }

lodge_ns_node_binary_func_impl(add, vec3, vec3)		{ return vec3_add(lhs, rhs); }
lodge_ns_node_binary_func_impl(mul, vec3, vec3)		{ return vec3_mult(lhs, rhs); }
//lodge_ns_node_binary_func_impl(div, vec3, vec3)		{ return vec3_div(lhs, rhs); }
lodge_ns_node_binary_func_impl(cross, vec3, vec3)	{ return vec3_cross(lhs, rhs); }
//lodge_ns_node_binary_func_impl(max, vec3, vec3)		{ return vec3_max(lhs, rhs); }
//lodge_ns_node_binary_func_impl(min, vec3, vec3)		{ return vec3_min(lhs, rhs); }

lodge_ns_node_binary_func_impl(add, i32, int32_t)	{ return lhs + rhs; }
//lodge_ns_node_binary_func_impl(sub, i32, int32_t)	{ return lhs - rhs; }
lodge_ns_node_binary_func_impl(mul, i32, int32_t)	{ return lhs * rhs; }
lodge_ns_node_binary_func_impl(div, i32, int32_t)	{ return lhs / rhs; }
lodge_ns_node_binary_func_impl(mod, i32, int32_t)	{ return lhs % rhs; }
lodge_ns_node_binary_func_impl(max, i32, int32_t)	{ return max(lhs, rhs); }
lodge_ns_node_binary_func_impl(min, i32, int32_t)	{ return min(lhs, rhs); }

//lodge_ns_node_binary_func_impl(add, i64, int64_t)	{ return lhs + rhs; }
//lodge_ns_node_binary_func_impl(sub, i64, int64_t)	{ return lhs - rhs; }
//lodge_ns_node_binary_func_impl(mul, i64, int64_t)	{ return lhs * rhs; }
//lodge_ns_node_binary_func_impl(div, i64, int64_t)	{ return lhs / rhs; }
//lodge_ns_node_binary_func_impl(mod, i64, int64_t)	{ return lhs % rhs; }
//lodge_ns_node_binary_func_impl(max, i64, int64_t)	{ return max(lhs, rhs); }
//lodge_ns_node_binary_func_impl(min, i64, int64_t)	{ return min(lhs, rhs); }

lodge_ns_node_binary_func_impl(add, f32, float)		{ return lhs + rhs; }
//lodge_ns_node_binary_func_impl(sub, f32, float)		{ return lhs - rhs; }
lodge_ns_node_binary_func_impl(mul, f32, float)		{ return lhs * rhs; }
lodge_ns_node_binary_func_impl(div, f32, float)		{ return lhs / rhs; }
lodge_ns_node_binary_func_impl(mod, f32, float)		{ return (float)fmod((double)lhs, (double)rhs); }
lodge_ns_node_binary_func_impl(max, f32, float)		{ return max(lhs, rhs); }
lodge_ns_node_binary_func_impl(min, f32, float)		{ return min(lhs, rhs); }

//lodge_ns_node_binary_func_impl(add, f64, double)	{ return lhs + rhs; }
//lodge_ns_node_binary_func_impl(sub, f64, double)	{ return lhs - rhs; }
//lodge_ns_node_binary_func_impl(mul, f64, double)	{ return lhs * rhs; }
//lodge_ns_node_binary_func_impl(div, f64, double)	{ return lhs / rhs; }
lodge_ns_node_binary_func_impl(mod, f64, double)	{ return fmod(lhs, rhs); }
//lodge_ns_node_binary_func_impl(max, f64, double)	{ return max(lhs, rhs); }
//lodge_ns_node_binary_func_impl(min, f64, double)	{ return min(lhs, rhs); }


#define lodge_ns_node_cast_conv_impl(FROM_TYPE, FROM_C_TYPE, FROM_LODGE_TYPE, TO_TYPE, TO_C_TYPE, TO_LODGE_TYPE) \
	static bool node_conv_ ## FROM_TYPE ## _to_ ## TO_TYPE ## _configure(struct lodge_node *node) \
	{ \
		lodge_node_set_pins(node, \
			&(struct lodge_pins) { \
				.count = 1, \
				.pins = { \
					{ \
						.name = strview( #FROM_TYPE ), \
						.type = FROM_LODGE_TYPE, \
					} \
				}, \
			}, \
			&(struct lodge_pins) { \
				.count = 1, \
				.pins = { \
					{ \
						.name = strview( #TO_TYPE ), \
						.type = TO_LODGE_TYPE, \
					}, \
				} \
			} \
		); \
		return true; \
	} \
	\
	static bool node_conv_ ## FROM_TYPE ## _to_ ## TO_TYPE ## _run(struct lodge_node *node) \
	{ \
		const FROM_C_TYPE *value = lodge_node_get_ ## FROM_TYPE (node, 0); \
		if(!value) { \
			return false; \
		} \
		lodge_node_set_ ## TO_TYPE (node, 0, (TO_C_TYPE)(*value)); \
		return true; \
	}

lodge_ns_node_cast_conv_impl(f32, float, LODGE_TYPE_F32, i32, int32_t, LODGE_TYPE_I32);
lodge_ns_node_cast_conv_impl(i32, int32_t, LODGE_TYPE_I32, f32, float, LODGE_TYPE_F32);
lodge_ns_node_cast_conv_impl(f64, double, LODGE_TYPE_F64, f32, float, LODGE_TYPE_F32);
lodge_ns_node_cast_conv_impl(f32, float, LODGE_TYPE_F32, f64, double, LODGE_TYPE_F64);

static bool node_math_lerp_f32_configure(struct lodge_node *node)
{
	lodge_node_set_pins(node,
		&(struct lodge_pins) {
			.count = 3,
			.pins = {
				{
					.name = strview("min"),
					.type = LODGE_TYPE_F32,
				},
				{
					.name = strview("max"),
					.type = LODGE_TYPE_F32,
				},
				{
					.name = strview("t"),
					.type = LODGE_TYPE_F32,
				}
			},
		},
		&(struct lodge_pins) {
			.count = 1,
			.pins =  {
				{
					.name = strview("out"),
					.type = LODGE_TYPE_F32
				}
			}
		}
	);
	return true;
}


static bool node_math_lerp_f32_run(struct lodge_node *node)
{
    const float *min_value = lodge_node_get_f32(node, 0);
	if(!min_value) {
		return false;
	}
    const float *max_value = lodge_node_get_f32(node, 1);
	if(!max_value) {
		return false;
	}
    const float *t_value = lodge_node_get_f32(node, 2);
	if(!t_value) {
		return false;
	}
    lodge_node_set_f32(node, 0, lerp1f(*min_value, *max_value, *t_value));
	return true;
}

static bool node_util_print_i32_configure(struct lodge_node *node)
{
	lodge_node_set_inputs(node, 1, (struct lodge_pin[]) {
		{
			.name = strview("i32"),
			.type = LODGE_TYPE_I32,
		}
	});
	return true;
}

static bool node_util_print_i32_run(struct lodge_node *node)
{
    for(lodge_pin_idx_t i = 0, count = node->inputs.count; i < count; i++) {
        const int32_t *value = lodge_variant_get_i32(lodge_node_get_value(node, i));

        if(value) {
            printf("pin%d=%d\n", i, *value);
        } else {
            printf("pin%d=NULL\n", i);
        }
    }
	return true;
}

static bool node_util_print_f32_configure(struct lodge_node *node)
{
	lodge_node_set_inputs(node, 1, (struct lodge_pin[]) {
		{
			.name = strview("f32"),
			.type = LODGE_TYPE_F32,
		}
	});
	return true;
}

static bool node_util_print_f32_run(struct lodge_node *node)
{
	for(lodge_pin_idx_t i = 0, count = node->inputs.count; i < count; i++) {
		const float *value = lodge_variant_get_f32(lodge_node_get_value(node, i));

		if(value) {
			printf("pin%d=%f\n", i, *value);
		} else {
			printf("pin%d=NULL\n", i);
		}
	}
	return true;
}

static bool node_vec3_make_configure(struct lodge_node* node)
{
	lodge_node_set_pins(node,
		&(struct lodge_pins) {
			.count = 3,
			.pins = {
				{
					.name = strview("x"),
					.type = LODGE_TYPE_F32,
				},
				{
					.name = strview("y"),
					.type = LODGE_TYPE_F32,
				},
				{
					.name = strview("z"),
					.type = LODGE_TYPE_F32,
				},
			}
		},
		&(struct lodge_pins) {
			.count = 1,
			.pins = {
				{
					.name = strview("vec3"),
					.type = LODGE_TYPE_VEC3,
				}
			}
		}
	);
	return true;
}

static bool node_vec3_make_run(struct lodge_node* node)
{
	const float *x = lodge_node_get_f32(node, 0);
	if(!x) {
		return false;
	}

	const float *y = lodge_node_get_f32(node, 1);
	if(!y) {
		return false;
	}

	const float *z = lodge_node_get_f32(node, 2);
	if(!z) {
		return false;
	}

	lodge_node_set_vec3(node, 0, vec3_make(*x, *y, *z));
	return true;
}

static bool node_vec3_get_x_configure(struct lodge_node* node)
{
	lodge_node_set_pins(node,
		&(struct lodge_pins) {
			.count = 1,
			.pins = {
				{
					.name = strview("vec3"),
					.type = LODGE_TYPE_VEC3,
				}
			}
		},
		&(struct lodge_pins) {
			.count = 1,
			.pins = {
				{
					.name = strview("x"),
					.type = LODGE_TYPE_F32,
				}
			}
		}
	);
	return true;
}

static bool node_vec3_get_x_run(struct lodge_node* node)
{
	const vec3 *v = lodge_node_get_vec3(node, 0);
	if(!v) {
		return false;
	}
	lodge_node_set_f32(node, 0, v->x);
	return true;
}

static bool node_vec3_break_configure(struct lodge_node* node)
{
	lodge_node_set_pins(node,
		&(struct lodge_pins) {
			.count = 1,
			.pins = {
				{
					.name = strview("vec3"),
					.type = LODGE_TYPE_VEC3,
				}
			}
		},
		&(struct lodge_pins) {
			.count = 3,
			.pins = {
				{
					.name = strview("x"),
					.type = LODGE_TYPE_F32,
				},
				{
					.name = strview("y"),
					.type = LODGE_TYPE_F32,
				},
				{
					.name = strview("z"),
					.type = LODGE_TYPE_F32,
				}
			}
		}
	);
	return true;
}

static bool node_vec3_break_run(struct lodge_node* node)
{
	const vec3 *v = lodge_node_get_vec3(node, 0);
	if(!v) {
		return false;
	}
	lodge_node_set_f32(node, 0, v->x);
	lodge_node_set_f32(node, 1, v->y);
	lodge_node_set_f32(node, 2, v->z);
	return true;
}

static bool node_vec3_get_y_configure(struct lodge_node* node)
{
	lodge_node_set_pins(node,
		&(struct lodge_pins) {
			.count = 1,
			.pins = {
				{
					.name = strview("vec3"),
					.type = LODGE_TYPE_VEC3,
				}
			}
		},
		&(struct lodge_pins) {
			.count = 1,
			.pins = {
				{
					.name = strview("y"),
					.type = LODGE_TYPE_F32,
				}
			}
		}
	);
	return true;
}

static bool node_vec3_get_y_run(struct lodge_node* node)
{
	const vec3 *v = lodge_node_get_vec3(node, 0);
	if(!v) {
		return false;
	}
	lodge_node_set_f32(node, 0, v->y);
	return true;
}

static bool node_vec3_get_z_configure(struct lodge_node* node)
{
	lodge_node_set_pins(node,
		&(struct lodge_pins) {
			.count = 1,
			.pins = {
				{
					.name = strview("vec3"),
					.type = LODGE_TYPE_VEC3,
				}
			}
		},
		&(struct lodge_pins) {
			.count = 1,
			.pins = {
				{
					.name = strview("z"),
					.type = LODGE_TYPE_F32,
				}
			}
		}
	);
	return true;
}

static bool node_vec3_get_z_run(struct lodge_node* node)
{
	const vec3 *v = lodge_node_get_vec3(node, 0);
	if(!v) {
		return false;
	}
	lodge_node_set_f32(node, 0, v->z);
	return true;
}

void lodge_node_types_default_register()
{
	lodge_node_type_register(strview("const::boolean"), node_const_boolean_configure, NULL);
	lodge_node_type_register(strview("const::u32"), node_const_u32_configure, NULL);
	lodge_node_type_register(strview("const::u64"), node_const_u64_configure, NULL);
	lodge_node_type_register(strview("const::i32"), node_const_i32_configure, NULL);
	lodge_node_type_register(strview("const::i64"), node_const_i64_configure, NULL);
	lodge_node_type_register(strview("const::f32"), node_const_f32_configure, NULL);
	lodge_node_type_register(strview("const::f64"), node_const_f64_configure, NULL);
	lodge_node_type_register(strview("const::vec2"), node_const_vec2_configure, NULL);
	lodge_node_type_register(strview("const::vec3"), node_const_vec3_configure, NULL);
	lodge_node_type_register(strview("const::vec4"), node_const_vec4_configure, NULL);
	lodge_node_type_register(strview("const::mat4"), node_const_mat4_configure, NULL);

	lodge_node_type_register(strview("bool::and"), node_binary_boolean_configure, node_binary_boolean_and_run);
	lodge_node_type_register(strview("bool::or"), node_binary_boolean_configure, node_binary_boolean_or_run);
	lodge_node_type_register(strview("bool::xor"), node_binary_boolean_configure, node_binary_boolean_xor_run);

	lodge_node_type_register(strview("vec3::make"), node_vec3_make_configure, node_vec3_make_run);
	lodge_node_type_register(strview("vec3::get::x"), node_vec3_get_x_configure, node_vec3_get_x_run);
	lodge_node_type_register(strview("vec3::get::y"), node_vec3_get_y_configure, node_vec3_get_y_run);
	lodge_node_type_register(strview("vec3::get::z"), node_vec3_get_z_configure, node_vec3_get_z_run);
	lodge_node_type_register(strview("vec3::break"), node_vec3_break_configure, node_vec3_break_run);

	lodge_node_type_register(strview("conv::i32_to_f32"), node_conv_i32_to_f32_configure, node_conv_i32_to_f32_run);
	lodge_node_type_register(strview("conv::f32_to_i32"), node_conv_f32_to_i32_configure, node_conv_f32_to_i32_run);
	lodge_node_type_register(strview("conv::f32_to_f64"), node_conv_f32_to_f64_configure, node_conv_f32_to_f64_run);
	lodge_node_type_register(strview("conv::f64_to_f32"), node_conv_f64_to_f32_configure, node_conv_f64_to_f32_run);

	lodge_node_type_register(strview("math::add::i32"), node_binary_i32_configure, node_binary_i32_add_run);
	lodge_node_type_register(strview("math::mul::i32"), node_binary_i32_configure, node_binary_i32_mul_run);
	lodge_node_type_register(strview("math::div::i32"), node_binary_i32_configure, node_binary_i32_div_run);
	lodge_node_type_register(strview("math::mod::i32"), node_binary_i32_configure, node_binary_i32_mod_run);
	lodge_node_type_register(strview("math::min::i32"), node_binary_i32_configure, node_binary_i32_min_run);
	lodge_node_type_register(strview("math::max::i32"), node_binary_i32_configure, node_binary_i32_max_run);

	lodge_node_type_register(strview("math::add::f32"), node_binary_f32_configure, node_binary_f32_add_run);
	lodge_node_type_register(strview("math::mul::f32"), node_binary_f32_configure, node_binary_f32_mul_run);
	lodge_node_type_register(strview("math::div::f32"), node_binary_f32_configure, node_binary_f32_div_run);
	lodge_node_type_register(strview("math::min::f32"), node_binary_f32_configure, node_binary_f32_min_run);
	lodge_node_type_register(strview("math::max::f32"), node_binary_f32_configure, node_binary_f32_max_run);
	lodge_node_type_register(strview("math::mod::f32"), node_binary_f32_configure, node_binary_f32_mod_run);
	lodge_node_type_register(strview("math::lerp::f32"), node_math_lerp_f32_configure, node_math_lerp_f32_run);
	lodge_node_type_register(strview("math::sin::f32"), node_unary_f32_configure, node_unary_f32_sin_run);
	lodge_node_type_register(strview("math::cos::f32"), node_unary_f32_configure, node_unary_f32_cos_run);

	lodge_node_type_register(strview("math::add::vec3"), node_binary_vec3_configure, node_binary_vec3_add_run);
	lodge_node_type_register(strview("math::mul::vec3"), node_binary_vec3_configure, node_binary_vec3_mul_run);
	lodge_node_type_register(strview("math::cross::vec3"), node_binary_vec3_configure, node_binary_vec3_cross_run);

	lodge_node_type_register(strview("math::mod::f64"), node_binary_f64_configure, node_binary_f64_mod_run);
	lodge_node_type_register(strview("math::sin::f64"), node_unary_f64_configure, node_unary_f64_sin_run);
	lodge_node_type_register(strview("math::cos::f64"), node_unary_f64_configure, node_unary_f64_cos_run);
	
	lodge_node_type_register(strview("util::print::i32"), node_util_print_i32_configure, node_util_print_i32_run);
	lodge_node_type_register(strview("util::print::f32"), node_util_print_f32_configure, node_util_print_f32_run);
}
